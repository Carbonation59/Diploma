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
    int64_t id_cdr;
    int64_t cur_time;
    int duration;
    string from;
    string to;
    int id_sw;
    int id_in_tr;
    int id_out_tr;
    bool is_start;
};

const int64_t INF = 1e9;
const double k_cf = 0.3;                        // учёт погрешности по времени создания CDR
const double m_cf = 0.7;                        // учёт погрешности по длительности звонка
const double q_cf = 36;                          // итоговый множитель
const int n = 10;                               // количество устройств в сети +1 (здесь их 9) (+1 для учёта 1 индексации)
//const int n_calls = 100;                         // количество звонков
const int64_t un_tm_stmp_delay = 1707609600000;    // примерное время в секундах с 01.01.1970 до 01.01.2024
const int64_t time_upper_bound = 15811200000;      // верхняя граница для времени в секундах (полгода)
const int64_t max_dur_call = 1800000;              // максимальная продолжительность звонка в секундах 
const int prob_of_loss_num = 0;                 // вероятность потери звонка (числитель)
const int prob_of_loss_den = 100;                // вероятность потери звонка (знаменатель) (разделено для удобства)
const int max_delay_time = 50;                   // максимальная задержка по времени создания между соседними CDR'ами
const int max_delay_dur = 30;                    // максимальная задержка по длительности звонка между соседними CDR'ами


void get_graph_info(vector<vector<int>> &g_list, vector<vector<pair<int, int>>> &g_graph, 
                    vector<vector<int64_t>> &min_dist, Poco::Data::Session& session);

int find_gmt(int a, Poco::Data::Session& session);
void generate_info(Poco::Data::Session& session, int n_calls);

#endif
