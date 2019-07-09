//
// Created by yezhuoyang on 2019-07-08.
//

#ifndef RISCV_NEWPARSER_HPP
#define RISCV_NEWPARSER_HPP

#include "transform.hpp"
#include "newriscv.hpp"
#include "const.h"
using namespace std;


class Parser {
private:
    Riscv* pointer;
    optype cutype;
    immtype itype;
    int OP;
    int imm;
    int PC;
    bool jump;
    int E1,E2,E3;
    int rs1 = 0;
    int rs2 = 0;
    int rd = 0;
public:
    Parser(Riscv* const R){
        PC=0x0;
        pointer=R;
    }
    void parse() {
        cutype = NONE;
        if (OP == _output) {
            cutype = PRINT;
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
    }

    void print() {
        switch (cutype) {
            case LUI: cout << name[LUI] << "  " << regname[rd] << ",0x" << hex << imm << endl;break;
            case AUIPC:break;
            case JAL:cout << name[JAL] << "  " << regname[rd] << "," << hex << PC + imm << endl;break;
            case JALR:cout << name[JALR] << endl;break;
            case BEQ:cout << name[BEQ] << "  " << regname[rs1] << "," << regname[rs2] << "  " << hex << PC + imm<< endl;break;
            case BNE:cout << name[BNE] << "  " << regname[rs1] << " " << regname[rs2] << "," << hex << PC + imm<< endl;break;
            case BLT:cout << name[BLT] << "  " << regname[rs1] << ", " << regname[rs2] << "  " << hex << PC + imm<< endl;break;
            case BGE:cout << name[BGE] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< PC + imm << endl;break;
            case BLTU:cout << name[BLTU] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< PC + imm << endl;break;
            case BGEU:cout << name[BGEU] << "  " << "  " << regname[rs1] << "  " << regname[rs2] << "," << hex<< PC + imm << endl;break;
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


    bool execute() {
        /*
         * Read the current instruction at PC.
         */
        pointer->read(PC, OP);
        //cout<<hex<<PC<<"  ";
        // pointer->visit(PC);
        parse();
        //print();
        jump = false;
        switch (cutype) {
            case LUI:
                E1=0;
                E1=E1|(imm&I_31_12);
                pointer->write_reg(rd,E1);
                break;
            case AUIPC:
                PC=PC+imm;
                pointer->write_reg(rd, PC);
                jump = true;
                break;
            case JAL:
                pointer->write_reg(rd, PC + 4);
                PC=PC+imm;
                jump = true;
                break;
            case JALR:
                pointer->write_reg(rd,PC + 4);
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                E1=E1&I_31_1;
                PC = E1;
                jump = true;
                break;
            case BEQ:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (E1==E2){
                    jump = true;
                    PC +=imm;
                }
                break;
            case BNE:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (E1!=E2) {
                    jump = true;
                    PC +=imm;
                }
                break;
            case BLT:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (E1 < E2) {
                    jump = true;
                    PC +=imm;
                }
                break;
            case BGE:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (E1 >= E2) {
                    jump = true;
                    PC += imm;
                }
                break;
            case BLTU:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if ((unsigned int)E1<(unsigned int)E2) {
                    jump = true;
                    PC+=imm;
                }
                break;
            case BGEU:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (!((unsigned int)E1<(unsigned int)E2)) {
                    jump = true;
                    PC+=imm;
                }
                break;
            case LB:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read(E1,E2);
                if(E2>0){
                    E2=(E2&I_7_0);
                }
                else{
                    E2=(E2|I_31_8);
                }
                pointer->write_reg(rd,E2);
                break;
            case LH:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read(E1,E2);
                if(E2>0){
                    E2=(E2&I_15_0);
                }
                else{
                    E2=(E2|I_31_16);
                }
                pointer->write_reg(rd,E2);
                break;
            case LW:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read(E1,E2);
                pointer->write_reg(rd,E2);
                break;
            case LBU:

                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read(E1,E2);
                E2=(E2&I_7_0);
                pointer->write_reg(rd,E2);
                break;

            case LHU:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read(E1,E2);
                E2=(E2&I_15_0);
                pointer->write_reg(rd,E2);
                break;
            case SB:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read_reg(rs2,E2);
                if(E2>0){
                    E2=(E2&I_7_0);
                }
                else{
                    E2=(E2|I_31_8);
                }
                pointer->write(E1,E2);
                break;
            case SH:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read_reg(rs2, E2);
                if(E2>0){
                    E2=(E2&I_15_0);
                }
                else{
                    E2=(E2|I_31_16);
                }
                pointer->write(E1,E2);
                break;
            case SW:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->read_reg(rs2,E2);
                pointer->write(E1,E2);
                break;
            case ADDI:
                pointer->read_reg(rs1,E1);
                E1=E1+imm;
                pointer->write_reg(rd,E1);
                break;
            case SLTI:
                pointer->read_reg(rs1,E1);
                if (E1 < imm)pointer->write_reg(rd, 1);
                else pointer->write_reg(rd, 0);
                break;
            case SLTIU:
                pointer->read_reg(rs1,E1);
                if ((unsigned int)E1<(unsigned int)imm)pointer->write_reg(rd, 1);
                else pointer->write_reg(rd, 0);
                break;
            case XORI:
                pointer->read_reg(rs1,E1);
                E1=E1^imm;
                pointer->write_reg(rd,E1);
                break;
            case ORI:
                pointer->read_reg(rs1,E1);
                E1=E1|imm;
                pointer->write_reg(rd,E1);
                break;
            case ANDI:
                pointer->read_reg(rs1,E1);
                E1=E1&imm;
                pointer->write_reg(rd,E1);
                break;
            case SLLI:
                pointer->read_reg(rs1,E1);
                E2=(I_4_0&imm);
                E1=(E1<<E2);
                pointer->write_reg(rd,E1);
                break;
            case SRLI:
                pointer->read_reg(rs1,E1);
                E2=(I_4_0&imm);
                E1=(E1>>E2);
                E1=(E1&(I_31_k_0(E2)));
                pointer->write_reg(rd,E1);
                break;

            case SRAI:
                pointer->read_reg(rs1,E1);
                E2=(I_4_0&imm);
                E1=(E1>>E2);
                if(E1>0){
                     E1=(E1&I_31_k_0(E2));
                }
                else{
                     E1=(E1|I_31_k_1(E2));
                }
                pointer->write_reg(rd,E1);
                break;
            case ADD:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E1=E1+E2;
                pointer->write_reg(rd,E1);
                break;
            case SUB:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E1=E1-E2;
                pointer->write_reg(rd,E1);
                break;
            case SLL:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E3=(E2&(I_4_0));
                E1=(E1<<E3);
                pointer->write_reg(rd,E1);
                break;
            case SLT:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (E1 < E2)pointer->write_reg(rd, 1);
                else pointer->write_reg(rd, 0);
                break;
            case SLTU:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                if (((unsigned int)E1<(unsigned int)E2))pointer->write_reg(rd, 1);
                else pointer->write_reg(rd, 0);
                break;
            case XOR:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E1=(E1^E2);
                pointer->write_reg(rd,E1);
                break;
            case SRL:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E3=(E2&I_4_0);
                E1=(E1>>E3);
                pointer->write_reg(rd,E1);
                break;
            case SRA:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E3=(E2&I_4_0);
                E1=(E1>>E3);
                if(E1>0){
                    E1=(E1&(I_31_k_0(E3)));
                }
                else{
                    E1=(E1|(I_31_k_1(E3)));
                }
                pointer->write_reg(rd,E1);
                break;
            case OR:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E1=E1|E2;
                pointer->write_reg(rd,E1);
                break;
            case AND:
                pointer->read_reg(rs1,E1);
                pointer->read_reg(rs2,E2);
                E1=E1&E2;
                pointer->write_reg(rd,E1);
                break;
            case NONE:
                cout << "WRONG!" << endl;
                break;
            case PRINT:
                cout << dec << pointer->print_ans() << endl;
        }
        if (!jump) PC += 4;
        if (cutype == PRINT||cutype==NONE) return false;
        else return true;
    }
    void set_op(const int&O){
        OP=O;
    }
};
#endif //RISCV_NEWPARSER_HPP
