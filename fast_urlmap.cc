/*
 * url_cache_map.cpp
 *
 *  Created on: 2016年9月18日
 *      Author: zhongxiankui
 */
#include <climits>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

#include "cache_urlmap.h"

using namespace mns;

template<class K>
NodeEntry<K>::NodeEntry(K key) {
    m_key = key;
    m_ts = 0;
    pre = NULL;
    next = NULL;
}

template<class K>
NodeEntry<K>::~NodeEntry() {
    printf("NodeEntry Release, key:[%s]\n", m_key.c_str());
}

template<class K, class V>
ValueEntry<K, V>::ValueEntry(V value, unsigned long long ts, NodeEntry<K>* node_ptr) {
    m_value = value;
    m_ts = ts;
    m_node_ptr = node_ptr;
}

template<class K, class V>
ValueEntry<K, V>::~ValueEntry() {

}

template<class K, class V>
TimeUrlMap<K, V>::TimeUrlMap(int max_size, long long interval) {
    new (this) TimeUrlMap(max_size, interval, 100);
}

template<class K, class V>
TimeUrlMap<K, V>::TimeUrlMap(int max_size, long long interval, int clear_size) {
    m_max_size = max_size;
    m_interval = interval;
    m_clear_size = clear_size;

    m_head_ptr = new NodeEntry<K>("head");
    m_tail_ptr = new NodeEntry<K>("tail");
    m_inner_map_ptr = new map<K, ValueEntry<K, V> >();
    m_head_ptr->next = m_tail_ptr;
    m_tail_ptr->pre = m_head_ptr;
    m_size = 0;
}

template<class K, class V>
TimeUrlMap<K, V>::TimeUrlMap(long long interval) {
    TimeUrlMap<string, string>(INT_MAX, interval);
}

template<class K, class V>
TimeUrlMap<K, V>::~TimeUrlMap() {

    NodeEntry<K>* node = m_head_ptr;
    NodeEntry<K>* tmp_ptr = NULL;
    while (NULL != node) {
        tmp_ptr = node->next;
        delete node;
        node = tmp_ptr;
    }

    if (NULL != m_inner_map_ptr) {
        delete m_inner_map_ptr;
    }
}

template<class K, class V>
bool TimeUrlMap<K, V>::insertAfterNode(NodeEntry<K>* insert_node_ptr, NodeEntry<K>* pre_node_ptr) {

    if (NULL == insert_node_ptr || NULL == pre_node_ptr) {
        return false;
    }

    pre_node_ptr->next->pre = insert_node_ptr;
    insert_node_ptr->next = pre_node_ptr->next;
    pre_node_ptr->next = insert_node_ptr;
    insert_node_ptr->pre = pre_node_ptr;

    return true;
}

template<class K, class V>
bool TimeUrlMap<K, V>::expireElement(long long last_ts) {
    NodeEntry<K>* node_ptr = m_tail_ptr->pre;
    NodeEntry<K>* del_node_ptr = node_ptr;
    int clear_count = 0;
    while ((m_size >= m_max_size || node_ptr->m_ts < last_ts) && node_ptr != m_head_ptr && clear_count < m_clear_size) {
        node_ptr->pre->next = m_tail_ptr;
        m_tail_ptr->pre = node_ptr->pre;
        del_node_ptr = node_ptr;
        node_ptr = node_ptr->pre;

        m_inner_map_ptr->erase(del_node_ptr->m_key);
        m_size--;
        delete (del_node_ptr);
        clear_count++;
    }

    return true;
}

template<class K, class V>
NodeEntry<K>* TimeUrlMap<K, V>::updateNode(K key, long long ts) {

    ValueEntry<K, V> value = m_inner_map_ptr->at(key);
    NodeEntry<K>* tmp_node_ptr = value.m_node_ptr;
    if (NULL == tmp_node_ptr) {
        return NULL;
    }

    tmp_node_ptr->m_ts = ts;
    value.m_node_ptr->pre->next = value.m_node_ptr->next;
    value.m_node_ptr->next->pre = value.m_node_ptr->pre;
    insertAfterNode(tmp_node_ptr, m_head_ptr);

    return tmp_node_ptr;
}

template<class K, class V>
bool TimeUrlMap<K, V>::put(const K& key, const V& value) {

    if (m_max_size <= 0) {
        return false;
    }

    long long now = gettimeMs();
    if (m_inner_map_ptr->find(key) != m_inner_map_ptr->end()) {
        NodeEntry<K>* node_ptr = updateNode(key, now);
        if (NULL == node_ptr) {
            return false;
        }

        ValueEntry<K, V> value_entry(value, now, node_ptr);
        m_inner_map_ptr->insert(std::pair<K, ValueEntry<K, V> >(key, value_entry));

        return true;
    } else {
        NodeEntry<K>* new_node_ptr = new NodeEntry<K>(key);
        if (NULL == new_node_ptr) {
            return false;
        }
        expireElement(now - m_interval);

        new_node_ptr->m_key = key;
        new_node_ptr->m_ts = now;
        insertAfterNode(new_node_ptr, m_head_ptr);
        m_size++;
        ValueEntry<K, V> value_entry(value, now, new_node_ptr);
        m_inner_map_ptr->insert(std::pair<K, ValueEntry<K, V> >(key, value_entry));

        return true;
    }

    return false;
}

template<class K, class V>
bool TimeUrlMap<K, V>::get(const K& key, V& value) {

    //obsolete expire element
    long long now = gettimeMs();
    long long last_ts = now - m_interval;
    expireElement(last_ts);

    typename map<K, ValueEntry<K, V> >::iterator mim_itr = m_inner_map_ptr->find(key);
    if (mim_itr != m_inner_map_ptr->end()) {
//        updateElement(key, now);
        ValueEntry<K, V> value_entry = mim_itr->second;
        value = value_entry.m_value;
        return true;
    } else {
        return false;
    }
}

template<class K, class V>
int TimeUrlMap<K, V>::size() {
    return m_size;
}

template<class K, class V>
NodeEntry<K>* TimeUrlMap<K, V>::getListHead() {
    return m_head_ptr;
}

template<class K, class V>
NodeEntry<K>* TimeUrlMap<K, V>::getListTail() {
    return m_tail_ptr;
}

template<class K, class V>
map<K, ValueEntry<K, V> >* TimeUrlMap<K, V>::getUrlMap() {
    return m_inner_map_ptr;
}

template<class K, class V>
int TimeUrlMap<K, V>::getClearSize() {
    return m_clear_size;
}


/*
void func143(int max_size, long long interval, int count) {
    long long start = gettimeMs();
    printf("start:[%lld]\n", start);

    TimeUrlMap<string, string>* mapPtr = new TimeUrlMap<string, string>(max_size, interval);
    printf("mapPtr:[%p]\n", mapPtr);

    int num = 0;
    while (num++ < count) {
        printf("start ===================================>\n");

        mapPtr->put("a", "A");
        printf("put ========================================\"a\", \"A\"\n");
        usleep(1000 * 1000);
        printf("sleep 1s\n");

        mapPtr->put("b", "B");
        printf("put ========================================\"b\", \"B\"\n");

        string AV = "";
        printf("get ========================================\"a\"\n");
        if (mapPtr->get("a", AV)) {
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("c", "C");
        printf("put ========================================\"c\", \"C\"\n");

        AV = "";
        printf("get ========================================\"a\"\n");
        if (mapPtr->get("a", AV)) {
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("d", "D");
        printf("put ========================================\"d\", \"D\"\n");

        AV = "";
        printf("get ========================================\"a\"\n");
        if (mapPtr->get("a", AV)) {
        }
        usleep(1000 * 1100);
        printf("sleep 1s\n");
        mapPtr->put("e", "E");
        printf("put ========================================\"e\", \"E\"\n");

        AV = "";
        printf("get ========================================\"a\"\n");
        if (mapPtr->get("a", AV)) {
        }
        string CV = "";
        printf("get ========================================\"c\"\n");
        if (mapPtr->get("c", CV)) {
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("f", "F");
        printf("put ========================================\"f\", \"F\"\n");

        NodeEntry<string>* head_ptr = mapPtr->getListHead();
        NodeEntry<string>* tail_ptr = mapPtr->getListTail();
        NodeEntry<string>* tmp_ptr = head_ptr->next;
        map<string, ValueEntry<string, string> >* map_url = mapPtr->getUrlMap();

        map<string, ValueEntry<string, string> >::iterator mu_itr = map_url->begin();
        printf("map_url size:[%d]\n", map_url->size());
        for (; mu_itr != map_url->end(); mu_itr++) {
            printf("key:[%s]-value:[%s]\n", mu_itr->first.c_str(), mu_itr->second.m_value.c_str());
        }

        while (tmp_ptr != tail_ptr) {
            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }

        printf("end ===================================>\n");
        usleep(1000 * 2000);
        printf("sleep 2s\n");
    }

    delete mapPtr;
}

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

#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

string int_to_string(int x) {
    stringstream strstream;
    strstream << x;
    return strstream.str();
}

void func142(int max_size, long long interval, int count, int inner_count) {
    long long start = gettimeMs();
    printf("start:[%lld]\n", start);

    TimeUrlMap<string, Test>* mapPtr = new TimeUrlMap<string, Test>(max_size, interval, 1);
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

        printf("map_url size:[%d]\n", map_url->size());
        map<string, ValueEntry<string, Test> >::iterator mu_itr = map_url->begin();
        for (; mu_itr != map_url->end(); mu_itr++) {
            printf("key:[%s]-value:[%d]\n", mu_itr->first.c_str(), mu_itr->second.m_value.a);
        }

        while (tmp_ptr != tail_ptr) {
            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }

        printf("end ===================================>\n");
        usleep(1000 * 2000);
        printf("sleep 2s\n");
    }

    delete mapPtr;
}

int main(int argc, char** argv) {
    printf("max_size:[%d], interval:[%lld], count:[%d]\n", atoi(argv[0]), atol(argv[1]), atoi(argv[3]));
    func142(atoi(argv[1]), atol(argv[2]), atoi(argv[3]), atoi(argv[4]));
}
*/
