/*
 * cache_url_map.h
 *
 *  Created on: 2016年9月18日
 *      Author: zhongxiankui
 */

#ifndef MILINK_COMMON_COMM_INCLUDE_CACHE_URLMAP_H_
#define MILINK_COMMON_COMM_INCLUDE_CACHE_URLMAP_H_

#include <sys/time.h>
#include <map>
using namespace std;

namespace mns{

long long gettimeMs()
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (long long)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}

template <class K>
class NodeEntry{

public:
    K m_key;
    unsigned long long m_ts;

public:

    NodeEntry<K>* pre;
    NodeEntry<K>* next;

public:
    NodeEntry(K key);
    ~NodeEntry();

};

template <class K, class V>
class ValueEntry{

public:
    unsigned long long m_ts;
    NodeEntry<K>* m_node_ptr;
    V m_value;

public:
    ValueEntry(V value, unsigned long long ts, NodeEntry<K>* node_ptr);
    ~ValueEntry();
};

template <class K, class V>
class TimeUrlMap{

public:

    TimeUrlMap(int max_size, long long interval);

    TimeUrlMap(int max_size, long long interval, int clear_size);

    TimeUrlMap(long long interval);

    ~TimeUrlMap();

    bool put(const K& key, const V& value);

    bool get(const K& key, V& value);

    int size();

    NodeEntry<K>* getListHead();

    NodeEntry<K>* getListTail();

    map<K, ValueEntry<K, V> >* getUrlMap();

    int getClearSize();

private:

    NodeEntry<K>* m_head_ptr;
    NodeEntry<K>* m_tail_ptr;

    int m_max_size;
    long long m_interval;
    int m_size;
    int m_clear_size;

    map<K, ValueEntry<K, V> >* m_inner_map_ptr;

    bool insertAfterNode(NodeEntry<K>* insert_node_ptr, NodeEntry<K>* pre_node_ptr);

    bool expireElement(long long last_ts);

    NodeEntry<K>* updateNode(K key, long long ts);

};

}


#endif /* MILINK_COMMON_COMM_INCLUDE_CACHE_URLMAP_H_ */
