//
// Created by yezhuoyang on 2019-07-08.
//

#ifndef RISCV_NEWPARSER_HPP
#define RISCV_NEWPARSER_HPP
#include "transform.hpp"
#include "const.h"
#include "newriscv.hpp"
#include "pipeline.hpp"
#include <list>
class pipeline;
using namespace std;
class Parser {
private:
    pipeline* pipe;
    optype cutype;
    immtype itype;
    stage S;
    /*
     * The remaining clock time to be executed.
     */
    int remain[5];
    bool delayed[5];
    int OP;
    int imm;
    int prepc;
    int PC;
    int E1,E2,E3;
    int rs1 = 0;
    int rs2 = 0;
    int rd = 0;
public:
    Parser(pipeline* const R=NULL){
        pipe=R;
        S=_END;
        cutype=optype(0);
    }

    void set_Pipe(pipeline* P){
        pipe=P;
    }

    void set_PC(const int&P){
        PC=P;
        cutype=LUI;
        prepc=P;
        for(int i=_IF;i<=_WB;++i)
            remain[i]=1;
        S=_IF;
    }

    void add_stall(){
        remain[S]++;
    }

    bool execute(){
         //print();

         //cout<<"EXE "<<stagename[S]<<endl;

         if(remain[S]>1){
             --remain[S];
             //cout<<stagename[S]<<"  "<<"stuck"<<endl;
             return true;
         }
         if(S==_END)return true;
         if(S==_WAIT)return true;
         switch(S){
            case _IF:
                      IF();break;
            case _ID:
                      ID();
                      lock();
                      //pipe->print_reg();
                      break;
            case _EX:
                      EX();
                      send_reg_forward();
                      send_mem_forward();
                      break;
            case _MEM:
                      MEM();
                      unlock_mem();
                      break;
            case _WB:
                      WB();
                      unlock_reg();
                      break;
        }
        S=stage((int)S+1);
        if(S==_END){
            if(cutype==PRINT) return false;
        }
        return true;
    }
    void ID() {
        cutype = NONE;
        if (OP == _output) {
            cutype = PRINT;
            load_reg();
            return;
        }
        switch (OP & _f5) {
            case _ty1:cutype = LUI;itype = _U;break;
            case _ty2:cutype = AUIPC;itype = _U;break;
            case _ty3:cutype = JAL;itype = _UJ;break;
            case _ty4:cutype = JALR;itype = _I;break;
            case _ty5:itype = _SB;
                switch (OP & _m3) {
                    case _r0:cutype = BEQ;break;
                    case _r1:cutype = BNE;break;
                    case _r4:cutype = BLT;break;
                    case _r5:cutype = BGE;break;
                    case _r6:cutype = BLTU;break;
                    case _r7:cutype = BGEU;break;
                }
                break;
            case _ty6:
                itype = _I;
                remain[_MEM]+=3;
                switch (OP & _m3) {
                    case _r0:cutype = LB;break;
                    case _r1:cutype = LH;break;
                    case _r2:cutype = LW;break;
                    case _r4:cutype = LBU;break;
                    case _r5:cutype = LHU;break;
                }
                break;
            case _ty7:
                itype = _S;
                remain[_MEM]+=3;
                switch (OP & _m3) {
                    case _r0:cutype = SB;break;
                    case _r1:cutype = SH;break;
                    case _r2:cutype = SW;break;
                }
                break;
            case _ty8:
                itype = _I;
                switch (OP & _m3) {
                    case _r0:cutype = ADDI;break;
                    case _r1:cutype = SLLI;break;
                    case _r2:cutype = SLTI;break;
                    case _r3:cutype = SLTIU;break;
                    case _r4:cutype = XORI;break;
                    case _r5:
                        switch (OP & _t7) {
                            case _f1:cutype = SRLI;break;
                            case _f2:cutype = SRAI;
                        }
                        break;
                    case _r6:cutype = ORI;break;
                    case _r7:cutype = ANDI;break;
                }
                break;
            case _ty9:
                itype = _R;
                switch (OP & _m3) {
                    case _r0:
                        switch (OP & _t7) {
                            case _f1:cutype = ADD;break;
                            case _f2:cutype=SUB;
                        }
                        break;
                    case _r1:
                        cutype = SLL;break;
                    case _r2:
                        cutype = SLT;break;
                    case _r3:
                        cutype = SLTU;break;
                    case _r4:
                        cutype = XOR;break;
                    case _r5:
                        switch (OP & _t7) {
                            case _f1:cutype = SRL;break;
                            case _f2:cutype = SRA;
                        }
                        break;
                    case _r6:cutype = OR;break;
                    case _r7:cutype = AND;break;
                }
                break;
        }
        set_all();
        load_reg();
    }
    void print() {
        if(S==_END||S==_WAIT){
             cout<<"WAIT"<<endl;return;
        }
        cout<<hex<<prepc<<" ";
        switch (cutype) {
            case LUI: cout << name[LUI] << "  " << regname[rd] << ",0x" << hex << imm << endl;break;
            case AUIPC:break;
            case JAL:cout << name[JAL] << "  " << regname[rd] << "," << hex << prepc + imm << endl;break;
            case JALR:cout << name[JALR] << endl;break;
            case BEQ:cout << name[BEQ] << "  " << regname[rs1] << "," << regname[rs2] << "  " << hex <<prepc + imm<< endl;break;
            case BNE:cout << name[BNE] << "  " << regname[rs1] << " " << regname[rs2] << "," << hex << prepc + imm<< endl;break;
            case BLT:cout << name[BLT] << "  " << regname[rs1] << ", " << regname[rs2] << "  " << hex << prepc + imm<< endl;break;
            case BGE:cout << name[BGE] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< prepc + imm << endl;break;
            case BLTU:cout << name[BLTU] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< prepc + imm << endl;break;
            case BGEU:cout << name[BGEU] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< prepc + imm << endl;break;
            case LB:cout << name[LB] << "  " << regname[rd] << "  " << imm << "(" << regname[rs1] << ")" << endl;break;
            case LH:cout << name[LH] << "  " << imm << "(" << regname[rs1] << ")" << endl;break;
            case LW:cout << name[LW] << "  " << dec << imm << "(" << regname[rs1] << ")" << endl;break;
            case LBU:cout << name[LBU] << "  " << regname[rd] << "  " << dec << imm << "(" << regname[rs1] << ")"<< endl;break;
            case LHU:cout << name[LHU] << "  " << imm << "(" << regname[rs1] << ")" << endl;break;
            case SB:cout << name[SB] << "  " << regname[rs2] << " " << imm << "(" << regname[rs1] << ")" << endl;break;
            case SH:cout << name[SH] << "  " << regname[rs2] << " " << imm << "(" << regname[rs1] << ")" << endl;break;
            case SW:cout << name[SW] << "  " << regname[rs2] << " " << dec << imm << "(" << regname[rs1] << ")"<< endl;break;
            case ADDI:cout << name[ADDI] << "  " << regname[rd] << " " << regname[rs1] << " " << dec << imm << endl;break;
            case SLTI:cout << name[SLTI] << endl;break;
            case SLTIU:cout << name[SLTIU] << endl;break;
            case XORI:cout << name[XORI] << endl;break;
            case ORI:cout << name[ORI] << endl;break;
            case ANDI:cout << name[ADDI] << "  " << regname[rs1] << " " << regname[rs1] << " " << imm << endl;break;
            case SLLI:cout << name[SLLI] << "   " << regname[rd] << " " << regname[rs1] << "  " << (imm & I_4_0) << endl;break;
            case SRLI:cout << name[SRLI] << "  " << regname[rd] << "," << regname[rs1] << "," << hex << "0x"<< (imm & I_4_0) << endl;break;
            case SRAI:cout << name[SRAI] << endl;break;
            case ADD:cout << name[ADD] << "  " << regname[rd] << " " << regname[rs1] << " " << regname[rs2] << endl;break;
            case SUB:cout << name[SUB] << "  " << regname[rd] << " " << regname[rs1] << " " << regname[rs2] << endl;break;
            case SLL:cout << name[SLL] << endl;break;
            case SLT:cout << name[SLT] << endl;break;
            case SLTU:cout << name[SLTU] << endl;break;
            case XOR:cout << name[XOR] << "  " << regname[rd] << " " << regname[rs1] << " " << regname[rs2] << endl;break;
            case SRL:cout << name[SRL] << endl;break;
            case SRA:cout << name[SRA] << endl;break;
            case OR:cout << name[OR] << "  " << regname[rd] << " " << regname[rs1] << " " << regname[rs2] << endl;break;
            case AND:cout << name[AND] << endl;break;
            case NONE:cout << "Invalid" << endl;break;
            case PRINT:cout << "_output!" << endl;
        }
    }
    void print_stage(){
            cout.setf(ios::left); //设置对齐方式为left
            cout.width(3); //设置宽度为7，不足用空格填充
            stage tmp=S;
            if(S==_END){
                cout<<"EMPTY"<<endl;return;
            }
            if(S==_WAIT){
                cout<<"WAIT"<<endl;return;
            }
            while(tmp!=S){
                tmp=stage(int(tmp)+1);
            }
            while(tmp!=_END){
                  cout<<remain[tmp]<<":"<<stagename[tmp]<<", ";
                  tmp=stage(int(tmp)+1);
            }
            cout<<endl;
    }
    void set_immI() {
        imm = 0;
        if (OP < 0)imm = imm | (I_31_11);
        imm = imm | ((OP >> 20) & I_10_1);
        if ((OP & I_20) > 0)imm = imm | I_0;
    }
    void set_immS() {
        imm = 0;
        if (OP < 0)imm = imm | I_31_11;
        imm = imm | ((OP >> 20) & I_10_5);
        imm = imm | ((OP >> 7) & I_4_0);
    }
    void set_immB() {
        imm = 0;
        if (OP < 0)imm = imm | I_31_12;
        if ((OP & I_7) > 0) imm = imm | I_11;
        imm = imm | ((OP >> 20) & (I_10_5));
        imm = imm | ((OP >> 7) & (I_4_1));
    }
    void set_immU() {
        imm = 0;
        imm = imm | (OP & I_31_12);
    }
    void set_immJ() {
        imm = 0;
        if (OP < 0) imm = imm | I_31_20;
        imm = imm | (I_19_12 & OP);
        if ((OP & I_20) > 0)imm = imm | I_11;
        imm = imm | ((OP >> 20) & I_10_1);
    }
    void set_rd() {
        rd = 0;
        rd = rd | (I_11_7 & OP);
        rd = rd >> 7;
    }
    void set_rs1() {
        rs1 = 0;
        rs1 = rs1 | (I_19_15 & OP);
        rs1 = rs1 >> 15;
    }
    void set_rs2() {
        rs2 = 0;
        rs2 = rs2 | (I_24_20 & OP);
        rs2 = rs2 >> 20;
    }
    void set_all() {
        if (itype == _I) set_immI();
        else if (itype == _S) set_immS();
        else if (itype == _SB) set_immB();
        else if (itype == _U) set_immU();
        else if (itype == _UJ) set_immJ();
        set_rd();
        set_rs1();
        set_rs2();
    }
    void IF(){
        pipe->read(PC, OP);
        prepc=PC;
        PC=PC+4;
    }
    void MEM(){
        switch (cutype){
            case LB:
                pipe->read(E1,E2);
                if(E2>0)
                    E2=(E2&I_7_0);
                else
                    E2=(E2|I_31_8);
                break;
            case LH:
                pipe->read(E1,E2);
                if(E2>0)E2=(E2&I_15_0);
                else E2=(E2|I_31_16);
                break;
            case LW:
                pipe->read(E1,E2);break;
            case LBU:
                pipe->read(E1,E2);E2=(E2&I_7_0);break;
            case LHU:
                pipe->read(E1,E2);E2=(E2&I_15_0);break;
            case SB:
                pipe->write(E1,E2);break;
            case SH:
                pipe->write(E1,E2);break;
            case SW:
                pipe->write(E1,E2);break;
        }
    }
    void WB(){
        switch (cutype) {
            case LUI:
                pipe->write_reg(rd,E1);break;
            case AUIPC:
                pipe->write_reg(rd, PC-4);break;
            case JAL:
                pipe->write_reg(rd,prepc+4);break;
            case JALR:
                pipe->write_reg(rd,prepc+4);break;
            case LB:
                pipe->write_reg(rd,E2);break;
            case LH:
                pipe->write_reg(rd,E2);break;
            case LW:
                pipe->write_reg(rd,E2);break;
            case LBU:
                pipe->write_reg(rd,E2);break;
            case LHU:
                pipe->write_reg(rd,E2);break;
            case ADDI:
                pipe->write_reg(rd,E1);break;
            case SLTI:
                if (E1 < imm)pipe->write_reg(rd, 1);
                else pipe->write_reg(rd, 0);
                break;
            case SLTIU:
                if ((unsigned int)E1<(unsigned int)imm)pipe->write_reg(rd, 1);
                 else pipe->write_reg(rd, 0);
                break;
            case XORI:
                pipe->write_reg(rd,E1);break;
            case ORI:
                pipe->write_reg(rd,E1);break;
            case ANDI:
                pipe->write_reg(rd,E1);break;
            case SLLI:
                pipe->write_reg(rd,E1);break;
            case SRLI:
                pipe->write_reg(rd,E1);break;
            case SRAI:
                pipe->write_reg(rd,E1);break;
            case ADD:
                pipe->write_reg(rd,E1);break;
            case SUB:
                pipe->write_reg(rd,E1);break;
            case SLL:
                pipe->write_reg(rd,E1);break;
            case SLT:
                if (E1 < E2)pipe->write_reg(rd, 1);
                else pipe->write_reg(rd, 0);
                break;
            case SLTU:
                  if (((unsigned int)E1<(unsigned int)E2))pipe->write_reg(rd, 1);
                   else pipe->write_reg(rd, 0);
                break;
            case XOR:
                 pipe->write_reg(rd,E1);break;
            case SRL:
                 pipe->write_reg(rd,E1);
                break;
            case SRA:
                pipe->write_reg(rd,E1);break;
            case OR:
                 pipe->write_reg(rd,E1);break;
            case AND:
                pipe->write_reg(rd,E1);break;
        }
    }
    void load_reg(){
        switch (cutype) {
            case JALR:
                pipe->read_reg(rs1,E1);break;
            case BEQ:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case BNE:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case BLT:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case BGE:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case BLTU:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case BGEU:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case LB:
                pipe->read_reg(rs1,E1);break;
            case LH:
                pipe->read_reg(rs1,E1);break;
            case LW:
                pipe->read_reg(rs1,E1);break;
            case LBU:
                pipe->read_reg(rs1,E1);break;
            case LHU:
                pipe->read_reg(rs1,E1);break;
            case SB:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SH:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2, E2);break;
            case SW:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case ADDI:
                pipe->read_reg(rs1,E1);break;
            case SLTI:
                pipe->read_reg(rs1,E1);break;
            case SLTIU:
                pipe->read_reg(rs1,E1);break;
            case XORI:
                pipe->read_reg(rs1,E1);break;
            case ORI:
                pipe->read_reg(rs1,E1);break;
            case ANDI:
                pipe->read_reg(rs1,E1);break;
            case SLLI:
                pipe->read_reg(rs1,E1);break;
            case SRLI:
                pipe->read_reg(rs1,E1);break;
            case SRAI:
                pipe->read_reg(rs1,E1);break;
            case ADD:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SUB:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SLL:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SLT:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SLTU:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case XOR:
                 pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SRL:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case SRA:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case OR:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case AND:
                pipe->read_reg(rs1,E1);pipe->read_reg(rs2,E2);break;
            case PRINT:{
                pipe->read_reg(10,E1);
            }
        }
    }
    void EX() {
        switch (cutype) {
            case LUI:   E1 = 0;E1 = E1 | (imm & I_31_12);break;
            case AUIPC: PC = PC-4+imm;break;
            case JAL:   PC = PC-4+imm;break;
            case JALR:  E1 = E1 + imm;E1 = E1 & I_31_1;PC = E1;break;
            case BEQ:   if (E1 == E2)PC=PC-4+imm;break;
            case BNE:   if (E1 != E2)PC=PC-4+imm;break;
            case BLT:   if (E1 < E2)PC=PC-4+imm;break;
            case BGE:   if (E1 >= E2)PC=PC-4+imm;break;
            case BLTU:  if ((unsigned int) E1 < (unsigned int) E2)PC=PC-4+imm;break;
            case BGEU:  if (!((unsigned int) E1 < (unsigned int) E2))PC=PC-4+imm;break;
            case LB:    E1 = E1 + imm;break;
            case LH:    E1 = E1 + imm;break;
            case LW:    E1 = E1 + imm;break;
            case LBU:   E1 = E1 + imm;break;
            case LHU:   E1 = E1 + imm;break;
            case SB:    E1 = E1 + imm;break;
            case SH:    E1 = E1 + imm;break;
            case SW:    E1 = E1 + imm;break;
            case ADDI:  E1 = E1 + imm;break;
            case XORI:  E1 = E1 ^ imm;break;
            case ORI:   E1 = E1 | imm;break;
            case ANDI:  E1 = E1 & imm;break;
            case SLLI:  E2 = (I_4_0 & imm);E1 = (E1 << E2);break;
            case SRLI:  E2 = (I_4_0 & imm);E1 = (E1 >> E2);E1 = (E1 & (I_31_k_0(E2)));break;
            case SRAI:  E2 = (I_4_0 & imm);E1 = (E1 >> E2);
                if (E1 > 0)
                    E1 = (E1 & I_31_k_0(E2));
                else
                    E1 = (E1 | I_31_k_1(E2));
                break;
            case ADD:   E1 = E1 + E2;break;
            case SUB:   E1 = E1 - E2;break;
            case SLL:   E3 = (E2 & (I_4_0));E1 = (E1 << E3);break;
            case XOR:   E1 = (E1 ^ E2);break;
            case SRL:   E3 = (E2 & I_4_0);E1 = (E1 >> E3);break;
            case SRA:   E3 = (E2 & I_4_0);E1 = (E1 >> E3);
                if (E1 > 0)
                    E1 = (E1 & (I_31_k_0(E3)));
                else
                    E1 = (E1 | (I_31_k_1(E3)));
                break;
            case OR:E1 = E1 | E2;break;
            case AND:E1 = E1 & E2;break;
            case PRINT:
                cout << dec << (((unsigned int) E1) & 255) << endl;
        }
    }


    void send_mem_forward(){
        switch (cutype){
            case SB:
                pipe->send_mem(E1,E2);break;
            case SH:
                pipe->send_mem(E1,E2);break;
            case SW:
                pipe->send_mem(E1,E2);break;
        }
    }
    /*
     * Send the signal of ALU to be stored in the register foward
     */
    void send_reg_forward(){
        switch (cutype) {
            case LUI:
                pipe->send_reg(rd,E1);break;
            case AUIPC:
                pipe->send_reg(rd,PC-4);break;
            case JAL:
                pipe->send_reg(rd,prepc+4);break;
            case JALR:
                pipe->send_reg(rd,prepc+4);break;
            case LB:
                pipe->send_reg(rd,E2);break;
            case LH:
                pipe->send_reg(rd,E2);break;
            case LW:
                pipe->send_reg(rd,E2);break;
            case LBU:
                pipe->send_reg(rd,E2);break;
            case LHU:
                pipe->send_reg(rd,E2);break;
            case ADDI:
                pipe->send_reg(rd,E1);break;
            case SLTI:
                if (E1 < imm)pipe->send_reg(rd, 1);
                else pipe->send_reg(rd, 0);
                break;
            case SLTIU:
                if ((unsigned int)E1<(unsigned int)imm)pipe->send_reg(rd, 1);
                else pipe->send_reg(rd, 0);
                break;
            case XORI:
                pipe->send_reg(rd,E1);break;
            case ORI:
                pipe->send_reg(rd,E1);break;
            case ANDI:
                pipe->send_reg(rd,E1);break;
            case SLLI:
                pipe->send_reg(rd,E1);break;
            case SRLI:
                pipe->send_reg(rd,E1);break;
            case SRAI:
                pipe->send_reg(rd,E1);break;
            case ADD:
                pipe->send_reg(rd,E1);break;
            case SUB:
                pipe->send_reg(rd,E1);break;
            case SLL:
                pipe->send_reg(rd,E1);break;
            case SLT:
                if (E1 < E2)pipe->send_reg(rd, 1);
                else pipe->send_reg(rd, 0);
                break;
            case SLTU:
                if (((unsigned int)E1<(unsigned int)E2))pipe->send_reg(rd, 1);
                else pipe->send_reg(rd,0);
                break;
            case XOR:
                pipe->send_reg(rd,E1);break;
            case SRL:
                pipe->send_reg(rd,E1);
                break;
            case SRA:
                pipe->send_reg(rd,E1);break;
            case OR:
                pipe->send_reg(rd,E1);break;
            case AND:
                pipe->send_reg(rd,E1);break;
        }
    }
    void set_op(const int&O){
        OP=O;
    }
    int cp(){
        return PC;
    }
    stage STAGE(){
        return S;
    }
    bool over(){
        return (S==_END);
    }
    bool waited(){
        return (S==_WAIT);
    }
    void clear(){
        imm=0;
        rs1 = 0;
        rs2 = 0;
        rd = 0;
        S=_WAIT;
    }
    void init(){
        S=_WAIT;

    }
    /*
     * Check whether the instruction is allow to be executed or not
     * return
     */
    void check(){
        switch (S){
            case _ID:
                if(check_reg()){
                    cout<<"reg is locked"<<endl;
                    remain[S]++;
                }
                break;
            case _MEM:
                if(check_mem())
                       remain[S]++;
                break;
        }
    }
    bool check_reg(){
        switch (cutype) {
            case JALR:
                return pipe->check_reg(rs1);
            case BEQ:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case BNE:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case BLT:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case BGE:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case BLTU:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case BGEU:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case LB:
                return pipe->check_reg(rs1);
            case LH:
                return pipe->check_reg(rs1);
            case LW:
                return pipe->check_reg(rs1);
            case LBU:
                return pipe->check_reg(rs1);
            case LHU:
                return pipe->check_reg(rs1);
            case SB:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SH:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SW:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case ADDI:
                return pipe->check_reg(rs1);
            case SLTI:
                return pipe->check_reg(rs1);
            case SLTIU:
                return pipe->check_reg(rs1);
            case XORI:
                return pipe->check_reg(rs1);
            case ORI:
                return pipe->check_reg(rs1);
            case ANDI:
                return pipe->check_reg(rs1);
            case SLLI:
                return pipe->check_reg(rs1);
            case SRLI:
                return pipe->check_reg(rs1);
            case SRAI:
                return pipe->check_reg(rs1);
            case ADD:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SUB:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SLL:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SLT:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SLTU:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case XOR:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SRL:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case SRA:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case OR:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case AND:
                return pipe->check_reg(rs1)||pipe->check_reg(rs2);
            case PRINT:{
                return pipe->check_reg(10);
            }
        }
    }
    /*
     *  Check whether a memory is locked or not.
     */
    bool check_mem() {
        switch (cutype) {
            case LB:
                return pipe->check_memory(E1);
            case LH:
                return pipe->check_memory(E1);
            case LW:
                return pipe->check_memory(E1);
            case LBU:
                return pipe->check_memory(E1);
            case LHU:
                return pipe->check_memory(E1);
            case SB:
                return pipe->check_memory(E1);
            case SH:
                return pipe->check_memory(E1);
            case SW:
                return pipe->check_memory(E1);
            default:
                return false;
        }
    }
    void lock(){
        lock_reg();
        lock_mem();
    }
    bool check_stage(){
        if(remain[S]>1) return false;
        if(S==_END) return false;
        if(S==_WAIT) return false;
        if(!pipe->check_stag(S)) return false;
        return true;
    }
    void lock_stage(){
        if(S==_END)return;
        if(S==_WAIT) return;
        pipe->lock_stag(S);
    }
    void lock_reg(){
        switch (cutype) {
            case LUI:
            case AUIPC:
            case JAL:
            case JALR:
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case ADDI:
            case SLTI:
            case SLTIU:
            case XORI:
            case ORI:
            case ANDI:
            case SLLI:
            case SRLI:
            case SRAI:
            case ADD:
            case SUB:
            case SLL:
            case SLT:
            case SLTU:
            case XOR:
            case SRL:
            case SRA:
            case OR:
            case AND:
                pipe->lock_reg(rd);break;
        }
    }
    void unlock_reg(){
        switch (cutype) {
            case LUI:
            case AUIPC:
            case JAL:
            case JALR:
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case ADDI:
            case SLTI:
            case SLTIU:
            case XORI:
            case ORI:
            case ANDI:
            case SLLI:
            case SRLI:
            case SRAI:
            case ADD:
            case SUB:
            case SLL:
            case SLT:
            case SLTU:
            case XOR:
            case SRL:
            case SRA:
            case OR:
            case AND:
                pipe->unlock_reg(rd);break;
        }
    }
    void lock_mem(){
        switch (cutype){
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case SB:
            case SH:
            case SW:
                pipe->lock_memory(E1);break;
        }
    }


    void unlock_mem(){
        switch (cutype){
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case SB:
            case SH:
            case SW:
                pipe->unlock_memory(E1);break;
        }
    }


    bool jump(){
        switch (cutype) {
            case AUIPC:
            case JAL:
            case JALR:
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
              return true;
        }
        return false;
    }
    int rem(const int&k){
        return remain[k];
    }



};
#endif //RISCV_NEWPARSER_HPP
