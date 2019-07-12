#include <iostream>
#include <string>


#include "const.h"
#include "pipeline.hpp"
#include "newparser.hpp"
#include "transform.hpp"
#include "newriscv.hpp"
#include "riscv.hpp"
#include "parser.hpp"
using namespace std;
const char* testname="/Users/yezhuoyang/Desktop/ppca/data/testcases/array_test1.data";
int S[maxsize];
int S0[maxsize];
#include "transform.hpp"

void print(Parser* flow);
int PC;
int T;
bool EX[pipenum];
int  seq[5];


void init(Parser* flow , pipeline&P){
    PC=0x00;
    T=0;
    for(int i=0;i<pipenum;++i){
        ++T;
        seq[i]=T;
        flow[i].set_Pipe(&P);
        flow[i].set_PC(PC);
        PC=PC+4;
    }
    //print(flow);
    cout<<endl;
}



bool execute(Parser* flow, pipeline& P){

     bool jump=false;
     /*
      * IF a certain stage is stucked, the following stages are all stucked
      */
    for(int i=0;i<pipenum;++i){
        if(flow[i].rem(flow[i].STAGE())>1){
              for(int j=0;j<pipenum;++j){
                  if(j==i)continue;
                  flow[j].add_stall();
              }
              break;
        }
    }

     for(int i=0;i<pipenum;++i){
         P.unlock_stag(i);
     }


    /*
     * Check whether an instruction has the same stage with any of the previous instruction
     */
    for(int i=0;i<pipenum;++i){
        if(flow[i].check_stage()){
            flow[i].add_stall();
            //cout<<i<<"  STAGE "<<stagename[flow[i].STAGE()]<<" locked"<<endl;
        }
        else
            flow[i].lock_stage();
        EX[i]=false;
    }



    /*
     * Check whether an jump occurs or not
     */
    for(int i=0;i<pipenum;++i) {
            if(flow[i].over()||flow[i].waited())continue;
            if(flow[i].jump()&&flow[i].STAGE()>_IF){
                //cout<<"jump"<<endl;
                for(int j=0;j<pipenum;++j){
                    if(seq[j]>seq[i]) flow[j].clear();
                }
                flow[i].execute();
                EX[i]=true;
                jump=true;
                if(flow[i].over()){
                    PC=flow[i].cp();

                    flow[i].print();
                    cout<<"NEW INS "<<PC<<endl;
                    //cout<<"jump to"<<PC<<endl;
                    jump=false;
                    EX[i]=false;
                    flow[i].clear();
                }
            }
    }


    /*
     * First run all instruction in the process of _EX, under which the data of register and memory is pushed forward.
     */
    for(int i=0;i<pipenum;++i){
        if(flow[i].STAGE()==_EX&&!EX[i]){
            flow[i].execute();
            EX[i]=true;
        }
    }



    /*
     *
     */
    for(int i=0;i<pipenum;++i){
        if(EX[i]) continue;
        if((flow[i].over()||flow[i].waited())&&!jump){
                    cout<<"NEW INS "<<PC<<endl;
                    if(flow[i].over())flow[i].print();

                    //cout<<"NEW INS "<<PC<<endl;
                    //flow[i].print();
                    flow[i].set_PC(PC);
                    seq[i]=T++;
                    PC+=4;
                    continue;
        }
        if(!flow[i].execute()) {
            return false;
        }
    }
    return true;
}


void print(Parser* flow){
    for(int i=0;i<pipenum;++i){
        flow[i].print();
    }
    for(int i=0;i<pipenum;++i){
        flow[i].print_stage();
    }
}


int main(){


    Riscv R(testname,S);
    Parser flow[pipenum];
    pipeline P(&R);
    bool con=true;
    init(flow,P);
    int p=0;


    while(con){
        ++p;
        if(p>200) break;
        //cout<<"step  "<<p<<endl;
        print(flow);
        con=execute(flow,P);

        //cout<<endl;
    }




/*
   Riscv A(testname,S);
   pipeline P(&A);

   riscv A0(testname,S0);
   parser B0(&A0);


    Parser flow[pipenum];


   bool R=true;
   int p=1;
   int D;


   while(R){
        ++D;
        //if(D>100)break;
        p=0;
        while(p!=2){
            p=P.execute();
        }
        R=B0.execute();
        cout<<endl;
   }
*/



}