
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <string>
#include "include/cache_urlmap.h"
using namespace std;
using namespace cache;

class Test {
public:
    ~Test() {
//        printf("a:[%d] Release\n", a);
    }
    Test() {
//        printf("a:[%d] Construct\n", a);
    }
    int a;
};
string int_to_string(int x) {
    stringstream strstream;
    strstream << x;
    return strstream.str();
}
void func142(int max_size, long long interval, int count, int inner_count) {
    TimeUrlMap<long, Test>* mapPtr = new TimeUrlMap<long, Test>(max_size, interval, 1);
    long long start = mapPtr->getTimeMills();
    printf("start:[%lld]\n", start);
    printf("mapPtr:[%p]\n", mapPtr);
    int num = 0;
    while (num++ < count) {
        printf("start ===================================>\n");
        int cnt = 0;
        while (cnt++ < inner_count) {
            Test A;
            A.a = cnt;
//            string key = "pre_" + int_to_string(cnt);
            long key = cnt;
            mapPtr->put(key, A);
            printf("put k:[%lld],v:[%d]\n", key, A.a);
            usleep(100 * 1);
            //        printf("sleep 10 us\n");
            Test AV;
            if (cnt > 3) {
//                string key1 = "pre_" + int_to_string(cnt - 2);
                long key1 = cnt - 2;
                if (mapPtr->get(key1, AV)) {
//                    printf("k:[%s], v:[%d]\n", key1.c_str(), AV.a);
                    printf("k:[%lld], v:[%d]\n", key1, AV.a);
                }
            }
        }
        NodeEntry<long>* head_ptr = mapPtr->getListHead();
        NodeEntry<long>* tail_ptr = mapPtr->getListTail();
        NodeEntry<long>* tmp_ptr = head_ptr->next;
        map<long, ValueEntry<long, Test> >* map_url = mapPtr->getUrlMap();

        map<long, ValueEntry<long, Test> >::iterator mu_itr = map_url->begin();
        for (; mu_itr != map_url->end(); mu_itr++) {
//            printf("key:[%s]-value:[%d]\n", mu_itr->first.c_str(), mu_itr->second.m_value.a);
            printf("key:[%lld]-value:[%lld]\n", mu_itr->first, mu_itr->second.m_value.a);
        }
        while (tmp_ptr != tail_ptr) {
//            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            printf("key:[%lld], ts:[%lld]\n", tmp_ptr->m_key, tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }
        printf("map_url size:[%d]\n", map_url->size());
        printf("end ===================================>\n");
        usleep(1000 * 2000);
        printf("sleep 2s\n");
    }
    delete mapPtr;
}

void func143(int max_size, long long interval, int count, int inner_count) {
    TimeUrlMap<string, Test>* mapPtr = new TimeUrlMap<string, Test>(max_size, interval, 1);
    long long start = mapPtr->getTimeMills();
    printf("start:[%lld]\n", start);
    printf("mapPtr:[%p]\n", mapPtr);
    int num = 0;
    while (num++ < count) {
        printf("start ===================================>\n");
        int cnt = 0;
        while (cnt++ < inner_count) {
            Test A;
            A.a = cnt;
            string key = "pre_" + int_to_string(cnt);
            mapPtr->put(key, A);
            printf("put k:[%s],v:[%d]\n", key.c_str(), A.a);
            usleep(100 * 1);
            //        printf("sleep 10 us\n");
            Test AV;
            if (cnt > 3) {
                string key1 = "pre_" + int_to_string(cnt - 2);
                if (mapPtr->get(key1, AV)) {
                    printf("k:[%s], v:[%d]\n", key1.c_str(), AV.a);
                }
            }
        }
        NodeEntry<string>* head_ptr = mapPtr->getListHead();
        NodeEntry<string>* tail_ptr = mapPtr->getListTail();
        NodeEntry<string>* tmp_ptr = head_ptr->next;
        map<string, ValueEntry<string, Test> >* map_url = mapPtr->getUrlMap();
        map<string, ValueEntry<string, Test> >::iterator mu_itr = map_url->begin();
        for (; mu_itr != map_url->end(); mu_itr++) {
            printf("key:[%s]-value:[%d]\n", mu_itr->first.c_str(), mu_itr->second.m_value.a);
        }
        while (tmp_ptr != tail_ptr) {
            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }
        printf("map_url size:[%d]\n", map_url->size());
        printf("end ===================================>\n");
        usleep(1000 * 2000);
        printf("sleep 2s\n");
    }
    delete mapPtr;
}

int main(int argc, char** argv) {
    printf("max_size:[%d], interval:[%lld], count:[%d]\n", atoi(argv[0]), atol(argv[1]), atoi(argv[3]));
//    func142(atoi(argv[1]), atol(argv[2]), atoi(argv[3]), atoi(argv[4]));
    func143(atoi(argv[1]), atol(argv[2]), atoi(argv[3]), atoi(argv[4]));
}
