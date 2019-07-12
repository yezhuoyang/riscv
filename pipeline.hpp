//
// Created by yezhuoyang on 2019-07-09.
//

#ifndef RISCV_PIPELINE_HPP
#define RISCV_PIPELINE_HPP
#include "const.h"
#include <queue>




class pipeline{
            Riscv* pointer;
            //Parser flow[pipenum];
            int clock;
            queue<int> PClist;
            int RUN;
            /*
            * Whther a register is to be written in the following step
            */
            bool reg_lock[32];
            bool reg_stored[32];
            int  regbuffer[32];
            bool stag_lock[32];
            map<int,int> buffer;
            map<int,bool> mem_lock;
    public:

                 pipeline(Riscv* const R):pointer(R){
                     RUN=true;
                     int PC=0x0;
                     for(int i=0;i<32;++i){
                         reg_lock[i]=false;
                         reg_stored[i]=false;
                     }
                     for(int i=0;i<5;++i){
                         stag_lock[i]=false;
                     }
                 }

                 /*
                  * Check whether a register can be read at present
                  */
                bool check_reg(const int&k){
                    return reg_lock[k]&&!reg_stored[k];
                }


                void lock_reg(const int&k){
                    reg_lock[k]=true;
                }


                void unlock_reg(const int&k){
                    reg_lock[k]=false;
                }


                bool check_stag(const int&k){
                    return stag_lock[k];
                }



                void lock_stag(const int&k){
                    stag_lock[k]=true;
                }


                void unlock_stag(const int&k){
                    stag_lock[k]=false;
                }
                /*
                 * Check whether an instruction to read memory P is permitted.
                 * Note that it is permitted only when both requirements bellow are satisfied
                 * 1. The memory is not marked locked yet.
                 * 2. The data in the memory hasn't been sent to the buffer yet.
                */
                bool check_memory(const int&P){
                    if(P%4){
                        return (mem_lock.count(P/4)&&!buffer.count(P/4))||(mem_lock.count(P/4+1)&&!buffer.count(P/4+1));
                    }
                    else{
                        return  (mem_lock.count(P/4)&&!buffer.count(P/4));
                    }
                }

                void lock_memory(const int&P){
                    if(P%4){
                        mem_lock[P/4]=true;
                        mem_lock[P/4+1]=true;
                    }
                    else{
                        mem_lock[P/4]=true;
                    }
                }


                void unlock_memory(const int&P){
                    if(P%4){
                        mem_lock.erase(P/4);
                        mem_lock.erase(P/4+1);
                    }
                    else
                        mem_lock.erase(P/4);
                }


                void write_buffer(const int&P,const int D){
                    buffer[P/4]=D;
                }


                void write(const int& pos,const int&B){
                    if(buffer.count(pos/4))buffer.erase(pos/4);
                    pointer->write(pos,B);
                }


                void read(const  int& pos,int&B){
                     if(buffer.count(pos/4)){
                          B=buffer[pos/4];
                     }
                     else{
                          pointer->read(pos,B);
                     }
                }

                /*
                * Read the register p to the bitset B
                */
                void read_reg(const int& p,int &B) const{
                    if(!reg_lock)
                           pointer->read_reg(p,B);
                    else
                           B=regbuffer[p];
                }


                /*
                *  Write the bitset to the register p
                */
                void write_reg(const int& p,const int& B) {
                    pointer->write_reg(p, B);
                }


                /*
                 * Send the data of d d to the memory buffer
                 */
                void send_mem(const int& p,const int& d){
                    buffer[p]=d;
                }


                /*
                 * Send the data of a flow forward to the register at p after ALU
                 */
                void send_reg(const int& p,const int& d){
                    regbuffer[p]=d;
                    reg_stored[p]=true;
                }





};




#endif //RISCV_PIPELINE_HPP
