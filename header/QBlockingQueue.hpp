//
// Created by chen on 2022/7/3.
//

#ifndef UNTITLED_QBLOCKINGQUEUE_HPP
#define UNTITLED_QBLOCKINGQUEUE_HPP

#include <QMutex>
#include <QWaitCondition>
enum BlockingQueueEnum{
    WAIT,
    FASTFAIL,
    DELETEHEAD
};

template<class T> class QBlockingQueue {
public:
    explicit QBlockingQueue(int defaultSize = 1024){
        this->maxSize = defaultSize;
        ts = new T[defaultSize];
    }

    ~QBlockingQueue(){
        conditionWrite.notify_all();
        mutex.tryLock();
        mutex.unlock();
        if (ts!= nullptr)
            delete[] ts;
    }

    bool get(T* ptr , int size,BlockingQueueEnum flag){
        if (size > maxSize)
            return false;

        mutex.lock();
        if(size <= cursor)
        {
            memcpy(ptr,ts,size*sizeof(T));
            memcpy(ts,(void*)((UINT64)ts+size*sizeof(T)),(cursor-size)*sizeof(T));
            cursor-=size;
        } else
        {
            switch (flag) {
                case BlockingQueueEnum::WAIT:{
                    int sy = 0; //已经复制的个数
                    while(sy<size)
                    {
                        int cps = (size-sy)>cursor?cursor:(size-sy);
                        memcpy(ptr+sy*sizeof(T),ts,sizeof(T)*(cps));
                        memcpy(ts,(void*)((UINT64)ts+cps*sizeof(T)),(cursor-cps)*sizeof(T));
                        cursor-=cps;
                        sy+=cps;
                        if(sy<size)
                            conditionRead.wait(&mutex);
                    }
                    break;
                }
                case BlockingQueueEnum::FASTFAIL:{
                    mutex.unlock();
                    return true;
                }

            }
        }
        conditionWrite.notify_all();
        mutex.unlock();
        return true;
    }

    bool put(T* ptr,int size,BlockingQueueEnum flag) {
        mutex.lock();

        if (size>maxSize)
        {
            mutex.unlock();
            return false;
        }

        if (size <= maxSize-cursor)
        {
            memcpy((void*)((UINT64)ts+sizeof(T)*cursor),ptr,size*sizeof(T));
            cursor+=size;
        } else
        {
            switch (flag) {
                case BlockingQueueEnum::WAIT:{
                    int nSize = 0; //已经复制的个数
                    while (nSize < size)
                    {
                        memcpy((void*)((UINT64)ts+sizeof(T)*cursor),ptr+sizeof(T)*nSize,
                               size-nSize <= maxSize-cursor?sizeof(T)*(size-nSize):sizeof(T)*(maxSize-cursor));
                        nSize+=maxSize-cursor?size-nSize:maxSize-cursor;
                        cursor+=maxSize-cursor?size-nSize:maxSize-cursor;
                        conditionRead.notify_all();
                        if (nSize < size)
                            conditionWrite.wait(&mutex);
                    }

                    break;
                }
                case BlockingQueueEnum::DELETEHEAD:{
                    int sy = size - (maxSize-cursor);
                    memcpy(ts,(void*)((UINT64)ts+sizeof(T)*sy),(maxSize-sy)*sizeof(T));
                    cursor-=sy;
                    memcpy((void*)((UINT64)ts+sizeof(T)*cursor),ptr,size*sizeof(T));
                    cursor+=size;
                    break;
                }
                case BlockingQueueEnum::FASTFAIL:{

                    mutex.unlock();
                    return false;
                }
            }
        }

        conditionRead.notify_all();
        mutex.unlock();
        return true;
    }

    inline int getMaxSize() const{
        return this->maxSize;
    }

    inline int getCursor() const{
        return this->cursor;
    }

private:
    void* ts = nullptr;
    volatile int maxSize = 0;
    volatile int cursor = 0;
    QMutex mutex;
    QWaitCondition conditionWrite;
    QWaitCondition conditionRead;
};



#endif //UNTITLED_QBLOCKINGQUEUE_HPP
