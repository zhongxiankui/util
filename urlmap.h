#ifndef SRC_TESTWORLD_H_
#define SRC_TESTWORLD_H_

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
class Entry{

public:
    K m_key;
    unsigned long long m_ts;


public:

    Entry<K>* pre;
    Entry<K>* next;

    unsigned long long getTs();
    void setTs(long long value);

public:
    Entry(K key);

};

template <class K, class V>
class TimeUrlMap{

public:

    TimeUrlMap(int max_size, long long interval);

    TimeUrlMap(long long interval);

    ~TimeUrlMap();

    bool put(K key, V value);

    bool get(K key, V& value);

    int size();

    Entry<K>* getListHead();

    Entry<K>* getListTail();

    map<K, V>* getUrlMap();

private:

    Entry<K>* m_head_ptr;
    Entry<K>* m_tail_ptr;

    int m_max_size;
    long long m_interval;
    int m_size;

    map<K, V>* m_inner_map_ptr;

    bool insertAfterNode(Entry<K>* insert_node_ptr, Entry<K>* pre_node_ptr);

    bool expireElement(long long last_ts);

    bool updateElement(K key, long long ts);

};

}


#endif /* SRC_TESTWORLD_H_ */
