//
// Created by yezhuoyang on 2019-07-03.
//

#ifndef RISCV_TRANSFORM_HPP
#define RISCV_TRANSFORM_HPP
using namespace std;

#include <bitset>

/*
 * Compare two bitset as unsigned numbers.
 */

bool _less(const bitset<32>&A,const bitset<32> &B){
     int p=31;
     while(p>=0){
         if(A[p]<B[p]) return true;
         if(A[p]>B[p]) return false;
         --p;
     }
     return false;
}

bitset<32> add(const bitset<32> &A,const bitset<32> &B) {
    bitset<32> tmp;
    int carry = 0;
    for (int i = 0; i < 32; ++i) {
        tmp[i] = (A[i] + B[i] + carry) % 2;
        carry = (A[i] + B[i] + carry) / 2;
    }
    return tmp;
}
/*
 * Get the interger formed by the lower five bit of A
 */
int lower_five(const bitset<32> &A){
    int tmp=0;
    for(int i=4;i>=0;--i){
        tmp*=2;
        tmp+=A[i];
    }
    return tmp;
}
/*
 * Take the negation of B
 */
bitset<32> neg(const bitset<32>& B) {
    bitset<32> tmp = ~B;
    tmp = add(tmp, bitset<32>("00000000000000000000000000000001"));
    return tmp;
}


bitset<32> sub(const bitset<32> &A,const bitset<32> &B) {
    return add(A, neg(B));
}


/*
 * Transform a bitset of length 32 to int.
 */
void to_num(const bitset<32>& T,int& I){
    I=0;
    if(!T[31]){
        for(int i=30;i>=0;--i){
            I*=2;
            I+=T[i];
        }
    }
    else{
        bitset<32> L=neg(T);
        if(L[31]){
            I=-1;
            for(int i=30;i>=0;--i){
                I*=2;
            }
            return;
        }
        for(int i=30;i>=0;--i){
            I*=2;
            I+=L[i];
        }
        I*=-1;
    }
}
/*
 * Transform an int to a bitset.
 */
void to_bit(const int& I,bitset<32>& T) {
    int tmp;
    if (I >=0) {
        tmp = I;
        for (int i = 0; i <= 30; ++i) {
            T[i] = (tmp % 2);
            tmp /= 2;
        }
        T[31]=0;
    } else {
        tmp = -I;
        for (int i = 0; i <= 30; ++i) {
            T[i] = (tmp % 2);
            tmp /= 2;
        }
        if(tmp){
            T[31]=1;
            return;
        }
        T[31]=0;
        T = neg(T);
    }
}

int hex_to_num(const char& A) {
    if (A >= '0' && A <= '9') {
        return A - '0';
    } else {
        return A - 'A' + 10;
    }
}

void num_to_hex(const int& A,char & o) {
    if (A >= 0 && A <= 9) {
        o = A + '0';
    } else {
        o = A - 10 + 'A';
    }
}

/*
 * Transform a char to a bitset of length 4.
 */
void to_bit(const char& A,bitset<4>& B) {
    int tmp;
    if (A >= 'A') tmp = A - 'A' + 10;
    else tmp = A - '0';
    for (int i = 0; i < 4; ++i) {
        if (tmp % 2)B[i] = 1;
        else B[i] = 0;
        tmp /= 2;
    }
}




void print_bin(int k){
    bitset<32> B;
    to_bit(k,B);
    cout<<B<<endl;
}


void reverse(char tmp[9]){
    char A,B;
    for(int i=0;i<2;++i){
        A=tmp[2*i];
        B=tmp[2*i+1];
        tmp[2*i]=tmp[2*(3-i)];
        tmp[2*i+1]=tmp[2*(3-i)+1];
        tmp[2*(3-i)]=A;
        tmp[2*(3-i)+1]=B;
    }
}


/*
 * Transform a cstring of length 8, each char in the string a character in hex form, to an int.
 * The function is used when reading the file.
 */
int change_to_int(const char tmp[9]) {
    bitset<32> R;
    bitset<4> L;
    int N;
    for (int i = 0; i < 8; ++i) {
        to_bit(tmp[i], L);
        for (int j = 0; j < 4; ++j) {
            R[28 - 4 * i + j] = L[j];
        }
    }
    to_num(R, N);
    return N;
}
/*
 * Change a bitstring of length to a char list.
 */
void change_to_char(char tmp[9],const bitset<32>& R) {
    int q;
    for (int i = 0; i < 8; ++i) {
        q = 0;
        for (int j = 3; j >= 0; --j){
            q *= 2;
            q += R[28 - 4 * i + j];
        }
        if (q < 10) {
            tmp[i] = '0' + q;
        } else {
            tmp[i] = 'A' + q - 10;
        }
    }
    tmp[8] = '\0';
}
/*
 * Change an integer to a char list
 */
void change_to_char(char tmp[9],const int& R){
    bitset<32> T;
    to_bit(R,T);
    change_to_char(tmp,T);
}





int I_31_k_1(const int&k){
    return 0xffffffff-(1<<(31-k))+1;
}


int I_31_k_0(const int k){
    return ((1<<(32-k))-1);
}



#endif //RISCV_TRANSFORM_HPP
