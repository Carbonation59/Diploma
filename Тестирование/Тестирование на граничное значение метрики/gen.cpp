#include <string>
#include <iostream>
#include <exception>
#include "structs_and_consts.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>

using namespace std;

int n_calls;


void get_graph_info(vector<vector<int>> &g_list, vector<vector<pair<int, int>>> &g_graph, 
                    vector<vector<int64_t>> &min_dist, Poco::Data::Session& session){
    switch_info cur;                    
    Poco::Data::Statement select_graph_info(session);
    select_graph_info << "SELECT id_switch, id_out_trunk, id_remote_trunk_in, "
                      << "id_remote_switch, time_zone, distance FROM Switches",
                      Poco::Data::Keywords::into(cur.id),
                      Poco::Data::Keywords::into(cur.id_out_tr),
                      Poco::Data::Keywords::into(cur.id_rem_in_tr),
                      Poco::Data::Keywords::into(cur.id_rem_sw),
                      Poco::Data::Keywords::into(cur.tm_zone),
                      Poco::Data::Keywords::into(cur.distance),
                      Poco::Data::Keywords::range(0, 1);

    while (!select_graph_info.done()) {
        if(select_graph_info.execute()){
            g_list[cur.id].push_back(cur.id_rem_sw);
            g_graph[cur.id][cur.id_rem_sw] = {cur.id_out_tr, cur.id_rem_in_tr};
            min_dist[cur.id][cur.id_rem_sw] = cur.distance;
        }
    }
}

void floyd_warshell(vector<vector<int64_t>>& g, vector<vector<int>>& p) {
	int n = g.size();
	for (int k = 0;k < n;k++) {
		for (int u = 0;u < n;u++) {
			for (int v = 0;v < n;v++) {
				if (g[u][v] > g[u][k] + g[k][v]) {
					g[u][v] = g[u][k] + g[k][v];
					p[u][v] = k;
				}
			}
		}
	}
}

vector<int> find_path(int u, int v, const vector<vector<int>> &p) {
	if (p[u][v] == -1) {
		return { u, v };
	}
	vector<int> v1 = find_path(u, p[u][v], p);
	vector<int> v2 = find_path(p[u][v], v, p);
	v1.pop_back();
    int n = v2.size();
	for (int i = 0;i < n;i++) {
		v1.push_back(v2[i]);
	}
	return v1;
}

void generate_cds(vector<pair<cdr_info, int>> &reqs, vector<int> &start_points, vector<int> &end_points){
    int size_of_st = start_points.size();
    int size_of_end = end_points.size();
    for(int i = 0; i < n_calls; i++){
        pair<cdr_info, int> cur;
        cur.first.cur_time = un_tm_stmp_delay + rand() % time_upper_bound;
        cur.first.duration = rand() % max_dur_call;
        cur.first.from = "8";
        cur.first.to = "8";
        for(int j = 0; j < 10; j++){
            char tmp1 = rand() % 10 + '0';
            char tmp2 = rand() % 10 + '0';
            cur.first.from = cur.first.from + tmp1;
            cur.first.to = cur.first.to + tmp2;
        }
        int id1 = rand() % size_of_st;
        cur.first.id_sw = start_points[id1];
        cur.first.is_start = true;
        int id2 = rand() % size_of_end;
        cur.second = end_points[id2];
        if(cur.first.id_sw == cur.second){                                    // входящий и исходящий абонент совпали
            if(size_of_st == 1){
                id2 = (id2 + 1) % size_of_end;
                cur.second = end_points[id2];
            } else {
                id1 = (id1 + 1) % size_of_st;
                cur.first.id_sw = start_points[id1];
            }
        }
        reqs.push_back(cur);
    }
}

int find_gmt(int a, Poco::Data::Session& session){
    int res;

    Poco::Data::Statement select_gmt(session);
    select_gmt << "SELECT time_zone FROM Switches WHERE id_switch = $1\n",
    Poco::Data::Keywords::use(a),
    Poco::Data::Keywords::into(res);
    select_gmt << "LIMIT 1";
    select_gmt.execute();

    return res;
}

void generate_info(Poco::Data::Session& session, int n_calls1){
    srand(time(0));
    n_calls = n_calls1;

    cout << "creating tables\n";

    Poco::Data::Statement delete_stmt1(session);
    delete_stmt1 << "DROP TABLE IF EXISTS Switches";
    delete_stmt1.execute();

    Poco::Data::Statement delete_stmt2(session);
    delete_stmt2 << "DROP TABLE IF EXISTS CDRs";
    delete_stmt2.execute();

    Poco::Data::Statement create_stmt1(session);

    create_stmt1 << "CREATE TABLE Switches"
                 << "("
                 << "id INT NOT NULL,"
                 << "id_switch INT NOT NULL,"                          // конкретный коммутатор
                 << "id_out_trunk INT NOT NULL,"                       // исходящий от нас транк
                 << "id_remote_trunk_in INT NOT NULL,"                 // входящий транк на удалённом коммутаторе
                 << "id_remote_switch INT NOT NULL,"                   // удалённый коммутатор
                 << "time_zone INT NOT NULL,"                          // часовой пояс коммутатора
                 << "distance INT NOT NULL"                            // расстояние по выбранному пути
                 << ");";
    create_stmt1.execute();

    Poco::Data::Statement create_stmt2(session);

    create_stmt2 << "CREATE TABLE CDRs"
                 << "("
                 << "id BIGINT NOT NULL,"
                 << "cur_time BIGINT NOT NULL,"                        // время, когда звонок показался на коммутаторе
                 << "duration_time INT NOT NULL,"                      // продолжительность звонка в секундах
                 << "number_from CHAR(11) NOT NULL,"                   // звонящий абонент
                 << "number_to CHAR(11) NOT NULL,"                     // принимающий абонент
                 << "id_switch INT NOT NULL,"                          // коммутатор, через который прошёл сигнал
                 << "id_in_trunk INT NOT NULL,"                        // транк, по которому ушёл звонок
                 << "id_out_trunk INT NOT NULL,"                        // транк, по которому пришёл звонок
                 << "is_start BOOL NOT NULL"                           // является ли cdr первым в цепочке
                 << ");";                     
    create_stmt2.execute();

    cout << "table created\n";

    vector<switch_info> network = {
        {7, -1, 3, 1, 0, 1},
        {1, 1, 2, 3, 1, 15},
        {1, 2, 1, 2, 1, 7},
        {2, 2, 1, 4, 1, 9},
        {3, 4, -3, 8, 2, 1},
        {3, 1, 3, 5, 2, 20},
        {3, 3, 2, 4, 2, 3},
        {4, 3, 1, 6, 3, 8},
        {6, 2, 1, 5, 4, 10},
        {5, 2, -2, 9, 1, 1}
    };

    vector<int> start_points = { 7 };                                     // операторы, от которых приходит звонок
    vector<int> end_points = { 8, 9 };                                       // операторы, к которым уходит звонок

    int n_sw = network.size();
    switch_info cur;
    for(int i = 1 ; i <= n_sw ; i++){
        Poco::Data::Statement insert1(session);
        cur = network[i - 1];
        insert1 << "INSERT INTO Switches (id, id_switch,id_out_trunk, "
                << "id_remote_trunk_in, id_remote_switch, time_zone, distance) "
                << "VALUES( $1, $2, $3, $4, $5, $6, $7)",
        Poco::Data::Keywords::use(i),
        Poco::Data::Keywords::use(cur.id),
        Poco::Data::Keywords::use(cur.id_out_tr),
        Poco::Data::Keywords::use(cur.id_rem_in_tr),
        Poco::Data::Keywords::use(cur.id_rem_sw),
        Poco::Data::Keywords::use(cur.tm_zone),
        Poco::Data::Keywords::use(cur.distance);

        insert1.execute();
    }

    cout << "switches-info inserted\n";

    vector<vector<int>> g_list(n);                                                  // в данной части алгоритма не испольуется                                                 
    vector<vector<pair<int, int>>> g_graph(n, vector<pair<int, int>> (n, {-1, -1})); // исходящий и входящий транки
    vector<vector<int64_t>> min_dist(n, vector<int64_t>(n, INF));                // минимальные расстояния между всеми коммутаторами
    get_graph_info(g_list, g_graph, min_dist, session);

    vector<vector<int>> parents(n, vector<int>(n, -1)); // вектор родителей для восстановления пути
    floyd_warshell(min_dist, parents);
    vector<vector<vector<int>>> pathes(n, vector<vector<int>>(n));
    for (int i = 1;i < n;i++) {
        for (int j = 1;j < n;j++) {
            pathes[i][j] = find_path(i, j, parents);
        }
    }

    vector<pair<cdr_info, int>> reqs;
    generate_cds(reqs, start_points, end_points);
    /*reqs = {
        { {-1, 100, 10, "89028765432", "89051234567", 7, -1, -1, true}, 9 },
        { {-1, 200, 15, "89028765432", "89051234567", 7, -1, -1, true}, 8 },
        { {-1, 300, 20, "89028765432", "89037777777", 7, -1, -1, true}, 6 },
        { {-1, 1000, 20, "89023333333", "89024444444", 7, -1, -1, true}, 2 },
        { {-1, 4602, 18, "89023333333", "89024444444", 2, 1, 2, false}, 9 },
    };*/

    int N = reqs.size();
    set<int64_t> cdr_id;                                              // все номера CDR'ов
    for(int i = 0 ; i < N ; i++){
        vector<int> cur_path = pathes[reqs[i].first.id_sw][reqs[i].second];
        int p_size = cur_path.size();
        bool prev_lost = false;                                      // был ли потерян прошлый CDR
        for(int j = 1 ; j < p_size - 1 ; j++){

            int gmt_1 = find_gmt(cur_path[j - 1], session);
            int gmt_2 = find_gmt(cur_path[j], session);

            int diffuse = (gmt_2 - gmt_1) * 3600 + (rand() % max_delay_time);
            reqs[i].first.cur_time = reqs[i].first.cur_time + diffuse;
            reqs[i].first.duration = reqs[i].first.duration - (rand() % max_delay_dur);
            reqs[i].first.duration = max(reqs[i].first.duration, 0);

            int64_t id = rand();
            while(cdr_id.find(id) != cdr_id.end()){
                id = rand();
            }
            cdr_id.insert(id);

            int num_of_loss = rand() % prob_of_loss_den;

            if(reqs[i].first.is_start || num_of_loss >= prob_of_loss_num || prev_lost){

                Poco::Data::Statement insert2(session);
                insert2 << "INSERT INTO CDRs (id, cur_time, duration_time, number_from, "
                        << "number_to, id_switch, id_in_trunk, id_out_trunk, is_start) "
                        << "VALUES( $1, $2, $3, $4, $5, $6, $7, $8, $9)",
                Poco::Data::Keywords::use(id),
                Poco::Data::Keywords::use(reqs[i].first.cur_time),
                Poco::Data::Keywords::use(reqs[i].first.duration),
                Poco::Data::Keywords::use(reqs[i].first.from),
                Poco::Data::Keywords::use(reqs[i].first.to),
                Poco::Data::Keywords::use(cur_path[j]),
                Poco::Data::Keywords::use(g_graph[cur_path[j - 1]][cur_path[j]].second),
                Poco::Data::Keywords::use(g_graph[cur_path[j]][cur_path[j + 1]].first),
                Poco::Data::Keywords::use(reqs[i].first.is_start);

                insert2.execute();

                prev_lost = false;

            } else {
                prev_lost = true;
            }

            reqs[i].first.is_start = false;
        }
    }

    cout << "CDRs-info inserted\n";
}
