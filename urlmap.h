#include "urlmap.h"

#include <climits>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

template <class K>
Entry<K>::Entry(K  key){
    m_key = key;
    m_ts = 0;
    pre = NULL;
    next = NULL;
}

template <class K>
Entry<K>::~Entry(){
}

template <class K>
unsigned long long Entry<K>::getTs(){
    return m_ts;
}

template<class K>
void Entry<K>::setTs(long long ts){
    m_ts = ts;
}

template<class K, class V>
TimeUrlMap<K, V>::TimeUrlMap(int max_size, long long interval){
    m_max_size = max_size;
    m_interval = interval;

    m_head_ptr = new Entry<K>("head");
    m_tail_ptr = new Entry<K>("tail");
    m_inner_map_ptr = new map<K, V>();
    m_head_ptr->next = m_tail_ptr;
    m_tail_ptr->pre = m_head_ptr;
    m_size = 0;
}

template<class K, class V>
TimeUrlMap<K, V>::TimeUrlMap(long long interval){
    TimeUrlMap<string, string>(INT_MAX,  interval);
}

template<class K, class V>
TimeUrlMap<K, V>::~TimeUrlMap(){

    Entry<K>* node = m_head_ptr;
    Entry<K>* tmp_ptr = NULL;
    while(NULL != node){
        tmp_ptr = node->next;
        delete node;
        node = tmp_ptr;
    }

    if(NULL != m_inner_map_ptr){
        delete m_inner_map_ptr;
    }
}

template<class K, class V>
bool TimeUrlMap<K, V>::insertAfterNode(Entry<K>* insert_node_ptr, Entry<K>* pre_node_ptr){

    if(NULL == insert_node_ptr || NULL == pre_node_ptr){
        return false;
    }

    pre_node_ptr->next->pre = insert_node_ptr;
    insert_node_ptr->next = pre_node_ptr->next;
    pre_node_ptr->next = insert_node_ptr;
    insert_node_ptr->pre = pre_node_ptr;

    return true;
}

template<class K, class V>
bool TimeUrlMap<K, V>::expireElement(long long last_ts){
    Entry<K>* node_ptr = m_tail_ptr->pre;
    Entry<K>* del_node_ptr = node_ptr;
    while(( m_size >= m_max_size || node_ptr->getTs() < last_ts) && node_ptr != m_head_ptr){
        node_ptr->pre->next = m_tail_ptr;
        m_tail_ptr->pre = node_ptr->pre;
        del_node_ptr = node_ptr;
        node_ptr = node_ptr->pre;

        m_inner_map_ptr->erase(del_node_ptr->m_key);
        m_size--;
        delete(del_node_ptr);
    }

    return true;
}

template<class K, class V>
bool TimeUrlMap<K, V>::updateElement(K key, long long ts){

    Entry<K>* node_ptr = m_head_ptr->next;
    while(node_ptr != m_tail_ptr){
        if(node_ptr->m_key == key){
            node_ptr->setTs(ts);
            Entry<K>* tmp_node_ptr = node_ptr;
            node_ptr->pre->next = node_ptr->next;
            node_ptr->next->pre = node_ptr->pre;
            insertAfterNode(tmp_node_ptr, m_head_ptr);

            return true;
        }
        node_ptr = node_ptr->next;
    }

    return false;
}

template<class K, class V>
bool TimeUrlMap<K, V>::put(K key, V value){

    if(m_max_size <= 0){
        return false;
    }

    long long now = gettimeMs();
    if(m_inner_map_ptr->find(key) != m_inner_map_ptr->end()){
        updateElement(key, now);
        m_inner_map_ptr->insert(std::pair<K, V>(key, value));

        return true;
    }else {
        Entry<K>* new_node_ptr = new Entry<K>(key);
        if(NULL == new_node_ptr){
            return false;
        }
        expireElement(now - m_interval);

        new_node_ptr->m_key = key;
        new_node_ptr->m_ts = now;
        insertAfterNode(new_node_ptr, m_head_ptr);
        m_size++;
        m_inner_map_ptr->insert(std::pair<K, V>(key, value));

        return true;
    }

    return false;
}

template<class K, class V>
bool TimeUrlMap<K, V>::get(K key, V& value){

    //obsolete expire element
    long long now = gettimeMs();
    long long last_ts = now - m_interval;
    expireElement(last_ts);

    if(m_inner_map_ptr->find(key) != m_inner_map_ptr->end()){
        updateElement(key, now);
        value = m_inner_map_ptr->at(key);
        return true;
    }else {
        return false;
    }
}

template<class K, class V>
int TimeUrlMap<K, V>::size(){
    return m_size;
}

template<class K, class V>
Entry<K>* TimeUrlMap<K, V>::getListHead(){
    return m_head_ptr;
}

template<class K, class V>
Entry<K>* TimeUrlMap<K, V>::getListTail(){
    return m_tail_ptr;
}

template<class K, class V>
map<K, V>* TimeUrlMap<K, V>::getUrlMap(){
    return m_inner_map_ptr;
}


/*
void func143(int max_size, long long interval, int count){
    long long  start =  gettimeMs();
    printf("start:[%lld]\n", start);

    TimeUrlMap<string, string>* mapPtr = new TimeUrlMap<string, string>(max_size, interval);
    printf("mapPtr:[%p]\n", mapPtr);

    int num = 0;
    while(num++ < count){
        printf("start ===================================>\n");

        mapPtr->put("a", "A");
        printf("put ========================================\"a\", \"A\"\n");
        usleep(1000 * 1000);
        printf("sleep 1s\n");

        mapPtr->put("b", "B");
        printf("put ========================================\"b\", \"B\"\n");

        string AV= "";
        printf("get ========================================\"a\"\n");
        if(mapPtr->get("a", AV)){
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("c", "C");
        printf("put ========================================\"c\", \"C\"\n");

        AV= "";
        printf("get ========================================\"a\"\n");
        if(mapPtr->get("a", AV)){
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("d", "D");
        printf("put ========================================\"d\", \"D\"\n");

        AV= "";
        printf("get ========================================\"a\"\n");
        if(mapPtr->get("a", AV)){
        }
        usleep(1000 * 1100);
        printf("sleep 1s\n");
        mapPtr->put("e", "E");
        printf("put ========================================\"e\", \"E\"\n");

        AV= "";
        printf("get ========================================\"a\"\n");
        if(mapPtr->get("a", AV)){
        }
        string CV= "";
        printf("get ========================================\"c\"\n");
        if(mapPtr->get("c", CV)){
        }
        usleep(1000 * 1000);
        printf("sleep 1s\n");
        mapPtr->put("f", "F");
        printf("put ========================================\"f\", \"F\"\n");

        Entry<string>* head_ptr = mapPtr->getListHead();
        Entry<string>* tail_ptr = mapPtr->getListTail();
        Entry<string>* tmp_ptr = head_ptr->next;
        map<string, string>* map_url = mapPtr->getUrlMap();

        map<string, string>::iterator mu_itr = map_url->begin();
        for(; mu_itr != map_url->end(); mu_itr++){
            printf("key:[%s]-value:[%s]\n", mu_itr->first.c_str(), mu_itr->second.c_str());
        }

        while(tmp_ptr != tail_ptr){
            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }

        printf("end ===================================>\n");
        usleep(1000 * 2000);
        printf("sleep 2s\n");
    }

    delete mapPtr;
}

    int main(int argc, char** argv){
     printf("max_size:[%d], interval:[%lld], count:[%d]\n", atoi(argv[0]), atol(argv[1]), atoi(argv[3]));
     func143(atoi(argv[1]), atol(argv[2]), atoi(argv[3]));
    }
*/
