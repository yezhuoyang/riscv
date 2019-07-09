//
// Created by yezhuoyang on 2019-07-09.
//

#ifndef RISCV_NEWRISCV_HPP
#define RISCV_NEWRISCV_HPP

#include "const.h"

using namespace std;
extern string regname[32];
struct ins{
    int time;
    int pos;
    ins(const int&a=0,const int&b=0):time(a),pos(b){}
    bool operator<(const ins&B) const{
        if(time>B.time)return true;
        if(time<B.time)return false;
        return (pos<B.pos);
    }
};


class Riscv{
    friend class Parser;
private:
    int* const storage;
    int  V[10000];
    int  reg[32];
    int  Size;
    FILE* infile;
    bitset<32> rev;
    bitset<32> R1;
    bitset<32> R2;
    bitset<32> tmp;
    int E1;
    int E2;
    int T;
public:

    Riscv(int* const S):storage(S){}
    void initial(){
        char tmp;
        char readin[9];
        char rev[9];
        /*
         * The number of characters already filled in readin
         */
        int cp=0;
        int pos=0;
        tmp=cin.get();
        for(int i=0;i<maxsize;++i)storage[i]=0;
        for(int i=0;i<10000;++i)V[i]=0;
        for(int i=0;i<32;++i)reg[i]=0;
        while(true){
            if(tmp==EOF){
                break;
            }
            while(tmp<'0'||(tmp>'9'&&tmp<'@')||tmp>'F'){
                tmp=cin.get();
                if(tmp==EOF){
                    break;
                }
            }
            if(tmp==EOF){
                break;
            }
            if(tmp=='@'){
                for(int i=0;i<8;++i){
                    readin[i]=cin.get();
                }
                pos=change_to_int(readin)/4;
                tmp=cin.get();
                tmp=cin.get();
                continue;
            }
            readin[cp++]=tmp;
            readin[cp++]=cin.get();
            if(cp==8){
                reverse(readin);
                storage[pos++]=change_to_int(readin);
                Size=pos;
                cp=0;
            }
            tmp=cin.get();
            if(tmp==EOF){
                break;
            }

        }
        for(int i=0;i<32;++i) reg[i]=0;
    }


    Riscv(const char* filename,int* const  S):storage(S){
        char tmp;
        char readin[9];
        char rev[9];
        /*
         * The number of characters already filled in readin
         */
        int cp=0;
        int pos=0;
        infile=fopen(filename,"r");
        if(infile==NULL){
            cout<<"Opening failed"<<endl;
            return;
        }
        tmp=fgetc(infile);
        for(int i=0;i<maxsize;++i)storage[i]=0;
        for(int i=0;i<10000;++i)V[i]=0;
        for(int i=0;i<32;++i)reg[i]=0;
        while(true){
            while(tmp<'0'||(tmp>'9'&&tmp<'@')||tmp>'F'){
                tmp=fgetc(infile);
                if(tmp==EOF){
                    break;
                }
            }
            if(tmp==EOF){
                break;
            }
            if(tmp=='@'){
                fgets(readin,9,infile);
                pos=change_to_int(readin)/4;
                tmp=fgetc(infile);
                continue;
            }
            readin[cp++]=tmp;
            readin[cp++]=fgetc(infile);
            if(cp==8){
                reverse(readin);
                storage[pos++]=change_to_int(readin);
                Size=pos;
                cp=0;
            }
            tmp=fgetc(infile);
        }
        for(int i=0;i<32;++i) reg[i]=0;
    }


    /*
     *  Write the bitset to the register p
     */
    void write_reg(const int& p,const int& B){
        if(!p) return;
        reg[p]=B;
    }
    void visit(const int&p){
        V[p/4]++;
    }
    /*
     * Read the register p to the bitset B
     */
    void read_reg(const int& p,int &B) const{
        B=reg[p];
    }


    void write(const int& pos,const int&B){
        if(pos%4){
            cout<<"unaligned!"<<endl;
        }
        else{
            storage[pos/4]=B;
        }
    }

    void read(const  int& pos,int&    B){
        if(pos%4){
            int rem=pos%4;
            E1=storage[pos/4];
            E2=storage[pos/4+1];
            B=0;
            B=(B|(E1>>(8*rem)));
            B=(B|((E2<<(8*(4-rem)))&I_31_k_1(8*rem)));
        }
        else{
            B=storage[pos/4];
        }
    }


    /*
     * Write a bitset of length 32 into the storage
     */
    void write(const int& pos,const bitset<32>&B){
        for(int i=0;i<4;++i){
            for(int j=0;j<8;++j){
                rev[8*i+j]=B[8*(3-i)+j];
            }
        }
        if(pos%4){
            to_bit(storage[pos/4],R1);
            to_bit(storage[(pos/4)+1],R2);
            int rem=pos%4;
            for(int i=3;i>=rem;--i){
                for(int j=0;j<8;++j){
                    R1[8*(i-rem)+j]=rev[8*i+j];
                }
            }
            for(int i=rem-1;i>=0;--i) {
                for (int j=0;j<8;++j){
                    R2[8*(3-i)+j]=rev[8*i+j];
                }
            }
            to_num(R1,storage[pos/4]);
            to_num(R2,storage[(pos/4)+1]);
        }
        else
        {
            to_num(rev,storage[pos/4]);
        }
    }


    void read(const int& pos,bitset<32>& B){
        if(pos%4){
            to_bit(storage[pos/4],R1);
            to_bit(storage[(pos/4)+1],R2);
            int rem=pos%4;
            for(int i=3;i>=rem;--i){
                for(int j=0;j<8;++j){
                    rev[8*i+j]=R1[8*(i-rem)+j];
                }
            }
            for(int i=rem-1;i>=0;--i) {
                for (int j=0;j<8;++j){
                    rev[8*i+j]=R2[8*(3-i)+j];
                }
            }
        }
        else
            to_bit(storage[pos/4],rev);
        for(int i=0;i<4;++i){
            for(int j=0;j<8;++j){
                B[8*i+j]=rev[8*(3-i)+j];
            }
        }
    }
    /*
     * Print the char list at pos.
     */
    void print(const int& pos){
        /*
         if(pos%4){
            cout<<"Not aligned!"<<endl;
            return;
         }
         */
        read(pos,tmp);
        to_num(tmp,T);
        char opt[9];
        change_to_char(opt,T);
        cout<<opt<<endl;
    }
    void print_reg() const{
        for(int i=0;i<32;++i){
            if(!reg[i])continue;
            cout<<regname[i]<<":"<<dec<<reg[i]<<" ";
        }
        cout<<endl;
    }
    void print_reg(int k){
        cout<<regname[k]<<":"<<dec<<reg[k]<<" "<<endl;
    }
    int print_ans() const{
        return ((unsigned int)reg[10])&255;
    }
    int getreg(const int&k){
        return reg[k];
    }
    int read_storage(int k){
        bitset<32> B;
        int tmp;
        read(k,B);
        to_num(B,tmp);
        return tmp;
    }
    ~Riscv(){
        //fclose(infile);
    }
};


#endif //RISCV_NEWRISCV_HPP
