#ifndef __STRUCTS_AND_CONSTS_H__
#define __STRUCTS_AND_CONSTS_H__

#include <string>
#include <iostream>
#include <exception>
#include "structs_and_consts.h"

#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>

using namespace std;

using namespace std;

struct switch_info{
    int id;
    int id_out_tr;
    int id_rem_in_tr;
    int id_rem_sw;
    int tm_zone;
    int distance;
};

struct cdr_info{
    int id_cdr;
    int64_t cur_time;
    int duration;
    string from;
    string to;
    int id_sw;
    int id_in_tr;
    int id_out_tr;
};

const int64_t INF = 1e9;
const int64_t k_cf = 1;
const int m_cf = 1;
const int64_t q_cf = 3;
const int n = 10;                  // количество устройств в сети +1 (здесь их 9)

void get_graph_info(vector<vector<int>> &g_list, vector<vector<pair<int, int>>> &g_graph, 
                    vector<vector<int64_t>> &min_dist, Poco::Data::Session& session);

int find_gmt(int a, Poco::Data::Session& session);
void generate_info(Poco::Data::Session& session);

#endif