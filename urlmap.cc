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
bool TimeUrlMap<K, V>::insertHead(Entry<K>* insert_node_ptr, Entry<K>* pre_node_ptr){

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
        printf("expireElement start 1, m_max_size:[%d], m_size:[%d], ts:[%lld],key:[%s], last_ts:[%lld]\n",
                m_max_size, m_size, node_ptr->getTs(), node_ptr->m_key.c_str(), last_ts);
        node_ptr->pre->next = m_tail_ptr;
        m_tail_ptr->pre = node_ptr->pre;
        del_node_ptr = node_ptr;
        node_ptr = node_ptr->pre;

        m_inner_map_ptr->erase(del_node_ptr->m_key);
        m_size--;
//        printf("del_node_ptr:[%p], key:[%s]\n", del_node_ptr, ((string)del_node_ptr->m_key).c_str());
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
            insertHead(tmp_node_ptr, m_head_ptr);
            printf("updateElement start1-1 key:[%s], now:[%lld]\n", key.c_str(), ts);

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

//    printf("put start\n");
    long long now = gettimeMs();
    if(m_inner_map_ptr->find(key) != m_inner_map_ptr->end()){
//        printf("put start1\n");
        updateElement(key, now);

        m_inner_map_ptr->insert(std::pair<K, V>(key, value));

        return true;
    }else {

//        printf("put start2\n");
        //obsolete expire element
        long long last_ts = now - m_interval;
//        printf("put start2-1-1 last_ts:[%lld],now:[%lld],m_interval:[%lld]\n", last_ts,now,m_interval);
//        printf("put start2-1-2 key:[%s],value:[%s]\n", key.c_str(), value.c_str());
        expireElement(last_ts);
//        printf("put start2-1\n");

        Entry<K>* new_node_ptr = new Entry<K>(key);
        if(NULL == new_node_ptr){
            return false;
        }

        new_node_ptr->m_key = key;
        new_node_ptr->m_ts = now;
        insertHead(new_node_ptr, m_head_ptr);
        printf("put start2-2 key:[%s],value:[%s], now:[%lld]\n", key.c_str(), value.c_str(), now);
//        printf("put start2-2\n");
        m_size++;
        m_inner_map_ptr->insert(std::pair<K, V>(key, value));
//        printf("put start3\n");
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



void func143(int max_size, long long interval){
    long long  start =  gettimeMs();
    printf("start:[%lld]\n", start);

    TimeUrlMap<string, string>* mapPtr = new TimeUrlMap<string, string>(max_size, interval);
    printf("mapPtr:[%p]\n", mapPtr);

    while(true){
        printf("start ===================================>\n");

        mapPtr->put("a", "A");
        usleep(1000 * 1000);
        mapPtr->put("b", "B");

        string AV= "";
        if(mapPtr->get("a", AV)){
            printf("1A:[%s]\n", AV.c_str());
        }
        usleep(1000 * 1000);
        mapPtr->put("c", "C");

        AV= "";
        if(mapPtr->get("a", AV)){
            printf("1A:[%s]\n", AV.c_str());
        }
        usleep(1000 * 1000);
        mapPtr->put("d", "D");
        AV= "";
        if(mapPtr->get("a", AV)){
            printf("1A:[%s]\n", AV.c_str());
        }
        usleep(1000 * 1100);
        mapPtr->put("e", "E");
        AV= "";
        if(mapPtr->get("a", AV)){
            printf("1A:[%s]\n", AV.c_str());
        }
        string CV= "";
        if(mapPtr->get("c", CV)){
            printf("1C:[%s]\n", CV.c_str());
        }
        usleep(1000 * 1000);
        mapPtr->put("f", "F");

        Entry<string>* head_ptr = mapPtr->getListHead();
        Entry<string>* tail_ptr = mapPtr->getListTail();
        Entry<string>* tmp_ptr = head_ptr->next;

        while(tmp_ptr != tail_ptr){
            printf("key:[%s], ts:[%lld]\n", tmp_ptr->m_key.c_str(), tmp_ptr->m_ts);
            tmp_ptr = tmp_ptr->next;
        }

        printf("end ===================================>\n");
        usleep(1000 * 2000);
    }
}
    
    int main(int argc, char** argv){
     printf("max_size:[%d], interval:[%lld]\n", atoi(argv[0]), atol(argv[1]));
     func143(atoi(argv[1]), atol(argv[2]));
    }
