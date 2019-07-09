#include <iostream>
#include <string>
#include "newparser.hpp"
#include "transform.hpp"
#include "newriscv.hpp"
#include "riscv.hpp"
#include "parser.hpp"

using namespace std;




int S[maxsize];



int main(){


   Riscv A(S);
   A.initial();
   Parser B(&A);
   bool R=true;
   while(R){
        R=B.execute();
   }


}