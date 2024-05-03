#include <string>
#include <iostream>
#include <exception>
#include "structs_and_consts.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>

using namespace std;

void find_cdr_by_id(int id, cdr_info& cur_cdr, Poco::Data::Session& session){
    Poco::Data::Statement select_cdr(session);
    select_cdr << "SELECT id, cur_time, duration_time, number_from, "
            << "number_to, id_switch, id_in_trunk, id_out_trunk FROM CDRs WHERE (id = $1)",
            Poco::Data::Keywords::use(id),
            Poco::Data::Keywords::into(cur_cdr.id_cdr),
            Poco::Data::Keywords::into(cur_cdr.cur_time),
            Poco::Data::Keywords::into(cur_cdr.duration),
            Poco::Data::Keywords::into(cur_cdr.from),
            Poco::Data::Keywords::into(cur_cdr.to),
            Poco::Data::Keywords::into(cur_cdr.id_sw),
            Poco::Data::Keywords::into(cur_cdr.id_in_tr),
            Poco::Data::Keywords::into(cur_cdr.id_out_tr);

    select_cdr.execute();
}

void find_next_cdr(cdr_info cur_cdr, pair<int, int> next_sw, pair<int, int64_t> &next_cdr, 
                    Poco::Data::Session& session, int gmt_1, int gmt_2){
    cdr_info tmp_cdr;

    Poco::Data::Statement select_next_cdr(session);
    select_next_cdr << "SELECT id, cur_time, duration_time, number_from, "
            << "number_to, id_switch, id_in_trunk, id_out_trunk FROM CDRs WHERE (number_from = $1) AND "
            << "(number_to = $2) AND (id_switch = $3) AND (id_in_trunk = $4)",
            Poco::Data::Keywords::use(cur_cdr.from),
            Poco::Data::Keywords::use(cur_cdr.to),
            Poco::Data::Keywords::use(next_sw.first),
            Poco::Data::Keywords::use(next_sw.second),
            Poco::Data::Keywords::into(tmp_cdr.id_cdr),
            Poco::Data::Keywords::into(tmp_cdr.cur_time),
            Poco::Data::Keywords::into(tmp_cdr.duration),
            Poco::Data::Keywords::into(tmp_cdr.from),
            Poco::Data::Keywords::into(tmp_cdr.to),
            Poco::Data::Keywords::into(tmp_cdr.id_sw),
            Poco::Data::Keywords::into(tmp_cdr.id_in_tr),
            Poco::Data::Keywords::into(tmp_cdr.id_out_tr);

    while (!select_next_cdr.done()) {
        if(select_next_cdr.execute()){
            int delta_t = abs( (tmp_cdr.cur_time - gmt_2 * 3600) - (cur_cdr.cur_time - gmt_1 * 3600) );
            int delta_d = abs( tmp_cdr.duration - cur_cdr.duration );
            int result = delta_t * k_cf + delta_d * m_cf;
            if(result <= q_cf){
                if(result < next_cdr.second){
                    next_cdr.first = tmp_cdr.id_cdr;
                    next_cdr.second = result;
                }
            }
        }
    }
}

bool check_next_trunk(cdr_info cur_cdr, Poco::Data::Session& session){
    int check_out = 0;

    Poco::Data::Statement select_trunk(session);
    select_trunk << "SELECT id_remote_trunk_in FROM Switches " 
            << "WHERE (id_switch = $1) AND (id_out_trunk = $2)",
    Poco::Data::Keywords::use(cur_cdr.id_sw),
    Poco::Data::Keywords::use(cur_cdr.id_out_tr),
    Poco::Data::Keywords::into(check_out);

    select_trunk.execute();

    if(check_out < 0){
        cout << "PARTNER WITH TRANK " << check_out << '\n';
        return true; 
    }

    return false;
}


int main(int argc, char *argv[]) 
{
    if (argc < 3)
    {
        cout << "Usage: sql_test address port\n";
        return 0;
    }
    string host(argv[1]);
    string port(argv[2]);

    cout << "connecting to:" << host <<":"<< port<< '\n';
    Poco::Data::PostgreSQL::Connector::registerConnector();
    cout << "connector registered" << '\n';

    string connection_str;
    connection_str = "user=admin password=root host="+host+" port="+port+" dbname=postgres";

    cout << connection_str << '\n';

    try
    {
        Poco::Data::Session session(
            Poco::Data::SessionFactory::instance().create(
                Poco::Data::PostgreSQL::Connector::KEY, connection_str));


        generate_info(session);                
        vector<vector<int>> g_list(n);
        vector<vector<pair<int, int>>> g_graph(n, vector<pair<int, int>> (n, {-1, -1})); // исходящий и входящий транки
        vector<vector<int64_t>> min_dist(n, vector<int64_t>(n, INF));                // минимальные расстояния между всеми коммутаторами
        get_graph_info(g_list, g_graph, min_dist, session);

        vector<int> start_of_path = {1, 6, 8, 11};
        int n1 = start_of_path.size();

        for(int i = 0 ; i < n1 ; i++){
            cdr_info cur_cdr;
            find_cdr_by_id(start_of_path[i], cur_cdr, session);

            set<int> checked_switches;                                     // коммутаторы, которые мы уже посетили
            checked_switches.insert(cur_cdr.id_cdr);

            int partner_in = 0;

            Poco::Data::Statement select_in(session);
            select_in << "SELECT id_out_trunk FROM Switches " 
                    << "WHERE (id_remote_trunk_in = $1) AND (id_remote_switch = $2)",
            Poco::Data::Keywords::use(cur_cdr.id_in_tr),
            Poco::Data::Keywords::use(cur_cdr.id_sw),
            Poco::Data::Keywords::into(partner_in);

            select_in.execute();

            cout << "Call number " << i + 1 << ": ";
            cout << "PARTNER WITH TRANK " << partner_in << " -> ";
            cout << cur_cdr.id_cdr << " -> ";

            while(true){
                pair<int, int64_t> next_cdr = {-1, (int64_t)1e14};
                pair<int, int> next_sw = {-1, -1};

                Poco::Data::Statement select_next(session);
                select_next << "SELECT id_remote_trunk_in, id_remote_switch FROM Switches " 
                        << "WHERE (id_switch = $1) AND (id_out_trunk = $2)",
                Poco::Data::Keywords::use(cur_cdr.id_sw),
                Poco::Data::Keywords::use(cur_cdr.id_out_tr),
                Poco::Data::Keywords::into(next_sw.second),
                Poco::Data::Keywords::into(next_sw.first);

                select_next.execute();

                if(next_sw.first == -1){
                    cout << "NEXT SWITCH DON'T EXIST\n";
                    break;
                }
                checked_switches.insert(next_sw.first);
                int gmt_1 = find_gmt(cur_cdr.id_sw, session);
                int gmt_2 = find_gmt(next_sw.first, session);

                find_next_cdr(cur_cdr, next_sw, next_cdr, session, gmt_1, gmt_2);

                if(next_cdr.first != -1){    
                    find_cdr_by_id(next_cdr.first, cur_cdr, session);
                    cout << cur_cdr.id_cdr << " -> ";
                    if(check_next_trunk(cur_cdr, session)){
                        break;
                    }    
                    continue;
                }

                // если запись была потеряна, то поиск продолжается на соседних коммутаторах
    
                cur_cdr.id_sw = next_sw.first;
                cur_cdr.id_in_tr = next_sw.second;
                cur_cdr.cur_time = cur_cdr.cur_time + (gmt_2 - gmt_1) * 3600 + (q_cf + 1) / 2;
                cur_cdr.duration = cur_cdr.duration - q_cf / 2;
                gmt_1 = gmt_2;
                int n2 = g_list[next_sw.first].size();
                for(int j = 0 ; j < n2 ; j++){

                    next_sw.first = g_list[next_sw.first][j];
                    if(checked_switches.find(next_sw.first) != checked_switches.end()){      // здесь уже были
                        continue;
                    }
                    checked_switches.insert(next_sw.first);
                    cur_cdr.id_out_tr = g_graph[cur_cdr.id_sw][next_sw.first].first;
                    next_sw.second = g_graph[cur_cdr.id_sw][next_sw.first].second;

                    gmt_2 = find_gmt(next_sw.first, session);

                    find_next_cdr(cur_cdr, next_sw, next_cdr, session, gmt_1, gmt_2);

                }

                if(next_cdr.first == -1){        
                    cout << "CAN'T RESTORE PATH\n";       
                    break;
                }
  
                find_cdr_by_id(next_cdr.first, cur_cdr, session);      
                cout << cur_cdr.id_cdr << " -> ";
                if(check_next_trunk(cur_cdr, session)){
                    break;
                }     
                
            }
        }
    }
    catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
    {
        cout << e.displayText() << '\n';
    }
    catch (Poco::Data::ConnectionFailedException &e)
    {
        cout << "err2\n";
    }
    catch (std::exception *ex)
    {
        cout << "err3\n";
    }
    cout << "end\n";
}