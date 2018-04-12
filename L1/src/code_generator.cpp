#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>

#include <code_generator.h>

using namespace std;

namespace L1{

    std::string clean_label(std::string label) {
        if (label.at(0) == char(':')) {
            label[0] = '_';
        }
        return label;
    }

    std::string register_map(std::string r) {
        if (r == "r10")
            return "r10b";
        if (r == "r11") 
            return "r11b";
        if (r ==  "r12")
            return "r12b";
        if (r == "r13")
            return "r13b";
        if (r ==  "r14")
            return "r14b";
        if (r == "r15")
            return "r15b";
        if (r == "r8")
            return "r8b";
        if (r == "r9")
            return "r9b";
        if (r == "rax")
            return "al";
        if (r == "rbp")
            return "bpl";
        if (r == "rbx")
            return "bl";
        if ( r == "rcx")
            return "cl";
        if ( r == "rdi")
            return "dil";
        if (r == "rdx")
            return "dl";
        if (r == "rsi")
            return "sil";
        return "";
    }
  


  void generate_code(Program p) {

    // std::ofstream outputFile;
    FILE *outputFile;

    std::cout << "opening file" << std::endl;
    outputFile = fopen("prog.S", "w");
    // outputFile.open("prog.S");

    p.entryPointLabel = clean_label(p.entryPointLabel);

    // Hard coded begining 
    fprintf(outputFile, ".text\n\t.globl go\ngo:\n\tpushq %%rbx\n\tpushq %%rbp\n\tpushq %%r12\n\tpushq %%r13\n\tpushq %%r14\n\tpushq %%r15\n\tcall %s\n\tpopq %%r15\n\tpopq %%r14\n\tpopq %%r13\n\tpopq %%r12\n\tpopq %%rbp\n\tpopq %%rbx\n\tretq\n", p.entryPointLabel.c_str());

    for (Function* F: p.functions) {
        F->name = clean_label(F->name);
        fprintf(outputFile, "%s:\n", F->name.c_str());
        for (Instruction* I: F->instructions) {

            // split instruction by words
            std::vector<std::string> result;
            std::istringstream iss(I->instruction);
            for(std::string s; iss >> s; )
                result.push_back(s);

            // initalize variables 
            std::string operation;
            std::string src;
            std::string dst;

            std::string arg1;
            std::string arg2;
            std::string extra_instruction;
            std::string inst;
            std::string operand;

            long offset;
            int idx;
            int r;


            switch (I->type) {

                //arithmetic
                case 0:
                   
                    // register/number to register
                    if (result.size() == 3) {

                        
                        if (result[1] == "+=") {
                            operation = "addq";
                        }
                        else if (result[1] == "-=") {
                            operation = "subq";
                        }
                        else if (result[1] == "*=") {
                            operation = "imulq";
                        }
                        else if (result[1] == "&=") {
                            operation = "andq";
                        }
                        else if (result[1] == ">>=") {
                            operation = "sarq";
                        }
                        else if (result[1] == "<<=") {
                            operation = "salq";
                        }

                        // determine if number of register
                        if (result[2][0] == 'r') {
                            src = '%' + result[2];
                        } else {
                            src = '$' + result[2];
                        }

                        dst = '%' + result[2];

                    }

                    else if (result.size() == 5) {

                        // 2 types of instructions: memory as source or as dest
                        idx = result[0] == "mem" ? 3 : 1;

                        if (result[idx] == "+=") {
                            operation = "addq";
                        }
                        else if (result[idx] == "-=") {
                            operation = "subq";
                        }
                        else if (result[idx] == "*=") {
                            operation = "imulq";
                        }
                        else if (result[idx] == "&=") {
                            operation = "andq";
                        }
                        else if (result[idx] == ">>=") {
                            operation = "sarq";
                        }
                        else if (result[idx] == "<<=") {
                            operation = "salq";
                        }

                        // memory is destination
                        if (idx == 3) {
                            // check if source is number/register
                            if (result[4][0] == 'r') {
                                src = '%' + result[4];
                            } else {
                                src = '$' + result[4];
                            }
                            // build destination string
                            dst = result[2] + '(' + '%' + result[1] + ')'; 
                        }
                        else {
                            // memory is the source (second arg in instruction)
                            src = result[4] + '(' + '%' + result[3] + ')'; 

                            // check if destination is number/register
                            if (result[4][0] == 'r') {
                                dst = '%' + result[4];
                            } else {
                                dst = '$' + result[4];
                            }
                        }
                    } 
                    // special case to allocate small registers
                    if (operation == "salq" || operation == "sarq") {
                        src = register_map(src);
                    } 
                    // write instruction using predefined variables
                    fprintf(outputFile, "\t%s %s, %s\n", operation.c_str(), src.c_str(), dst.c_str());

                    break;

                // store (run assignment code)
                case 3:
                // load (run assignment code)
                case 2: 
                // assignment
                case 1:
                    // constant for all assignments 
                    operation = "movq";

                    if (result.size() == 3) {

                        // determine if number of register
                        if (result[2][0] == 'r') {
                            src = '%' + result[2];
                        } 
                        else if (result[2][0] == ':'){
                            result[2] = clean_label(result[2]);
                            src = '$' + result[2];
                        }
                        else {
                            src = '$' + result[2];
                        }

                        // always a register in this case
                        dst = '%' + result[0];
                    }
                    // we have size of 5 
                    else if (result.size() == 5) {
                        if (result[0] == "mem") {
                            // determine if number of register
                            if (result[4][0] == 'r') {
                                src = '%' + result[4];
                            } 
                            else if (result[4][0] == ':'){
                                result[4] = clean_label(result[4]);
                                src = '$' + result[4];
                            }
                            else {
                                src = '$' + result[4];
                            }

                            // always a register in this case
                            dst = result[2] + '(' + '%' + result[1] + ')';


                        } else {
                            // determine if number of register
                            src = result[4] + '(' + '%' + result[3] + ')';
                        
                            // always a register in this case
                            dst = '%' + result[0];
                        }
                        
                    }
                    else {
                        src = result[6] + '(' + '%' + result[5] + ')';
                        dst = result[2] + '(' + '%' + result[1] + ')';
                    }


                    fprintf(outputFile, "\t%s %s, %s\n", operation.c_str(), src.c_str(), dst.c_str());
                    break;

                // instruction DNE
                case 4:
                    break;

                // compare and jump (cjmp)
                case 5:
                    operation = "cmpq";
                    operand = result[2];
                    if (result[3][0] != 'r') {

                        // both are numbers 
                        if (result[1][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[1].c_str()) < atoi(result[3].c_str());
                            }
                            else if (operand == "<=") {
                                r = atoi(result[1].c_str()) <= atoi(result[3].c_str());
                            }
                            else if (operand == "=") {
                                r = atoi(result[1].c_str()) == atoi(result[3].c_str());
                            }

                            if (r) {
                                result[4] = clean_label(result[4]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[4].c_str());
                            } else {
                                result[5] = clean_label(result[5]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[5].c_str());
                            }
                            break;
                        }

                        // only last one is a number

                        // negate
                        if (operand == "<") { operand = ">="; }
                        if (operand == "<=") { operand = ">="; }


                        // swap
                        arg1 = result[1];
                        arg2 = result[3];
                        result[1] = arg2;
                        result[3] = arg1;
                    }


                    if (operand == "<") { inst = "jl"; }
                    if (operand == "<=") { inst = "jle"; }
                    if (operand == ">") { inst = "jg"; }
                    if (operand == ">=") { inst = "jge"; }
                    if (operand == "=") { inst = "je"; }

                    if (result[1][0] == 'r') {
                        arg1 = '%' + result[1];
                    } else {
                        arg1 = '$' + result[1];
                    }

                    
                    arg2 = '%' + result[3];
                    result[4] = clean_label(result[4]);
                    result[5] = clean_label(result[5]);

                    fprintf(outputFile, "\t%s %s, %s\n\t%s %s\n\tjmp %s\n", operation.c_str(), arg1.c_str(), arg2.c_str(), inst.c_str(), result[4].c_str(), result[5].c_str() );

                    break;

                // goto
                case 6:
                    result[1] = clean_label(result[1]);
                    fprintf(outputFile, "%s %s\n", "jmp", result[1].c_str());
                    break;

                // return
                case 7:
                    offset = F->locals * 8;
                    if (offset) {
                        fprintf(outputFile, "\t%s $%ld, %%%s\n", "addq", offset, "rsp");
                    }
                    fprintf(outputFile, "\t%s\n", "retq");
                    break;

                // call
                case 8:

                    if (result[1][0] != 'r' && result[1][0] != ':') {
                        fprintf(outputFile, "\t%s %s %s\n", result[0].c_str(), result[1].c_str(), "# runtime system call");
                        break;
                    }

                    offset = ((atoi(result[2].c_str()) - 6 ) * 8) + 8;
                    if (offset < 8) { offset = 8; }

                    fprintf(outputFile, "\t%s $%ld, %%%s\n", "subq", offset, "rsp");

                    if (result[1][0] == 'r') {
                        fprintf(outputFile, "\t%s *%%%s\n", "jmp", "rdi");
                    } else {
                        result[1] = clean_label(result[1]);
                        fprintf(outputFile, "\t%s %s\n", "jmp", result[1].c_str());
                    }   

                    break;

                // lea 
                case 9:

                    fprintf(outputFile, "\t%s (%%%s, %%%s, %s), %%%s\n", "lea", result[2].c_str(), result[3].c_str(), result[4].c_str(), result[0].c_str());

                // compare assign
                case 10:

                    operation = "cmpq";

                    operand = result[3];
                    if (result[4][0] != 'r') {

                        // both are numbers 
                        if (result[2][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[2].c_str()) < atoi(result[4].c_str());
                            }
                            else if (operand == "<=") {
                                r = atoi(result[2].c_str()) <= atoi(result[4].c_str());
                            }
                            else if (operand == "=") {
                                r = atoi(result[2].c_str()) == atoi(result[4].c_str());
                            }

                            if (r) {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$1", result[0].c_str());
                            } else {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$0", result[0].c_str());
                            }
                            break;
                        }

                        // only last one is a number

                        // negate operands
                        if (operand == "<") { operand = ">="; }
                        if (operand == "<=") { operand = ">="; }


                        // swap results
                        arg1 = result[2];
                        arg2 = result[4];
                        result[2] = arg2;
                        result[4] = arg1;
                    }


                    if (operand == "<") { inst = "setl"; }
                    if (operand == "<=") { inst = "setle"; }
                    if (operand == ">") { inst = "setg"; }
                    if (operand == ">=") { inst = "setge"; }
                    if (operand == "=") { inst = "sete"; }

                    if (result[2][0] == 'r') {
                        arg1 = '%' + result[2];
                    } else {
                        arg1 = '$' + result[2];
                    }

                    
                    arg2 = '%' + result[4];

            
                    fprintf(outputFile, "\t%s %s, %s\n\t%s %%%s\n\tmovzbq %%%s, %%%s\n", operation.c_str(), arg1.c_str(), arg2.c_str(), inst.c_str(), register_map(result[0]).c_str(), register_map(result[0]).c_str(), result[0].c_str());

                    break;

                // label inst
                case 11:
                    result[0] = clean_label(result[0]);
                    fprintf(outputFile, "%s:\n", result[0].c_str());
                    break;

                // increment / decrement
                case 12:

                    if (result[0][3] == '+') {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "inc", result[0][0], result[0][1], result[0][2]);
                    } else {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "dec", result[0][0], result[0][1], result[0][2]);
                    }
                    break;

                default:
                    printf("%s\n", "error");

            }
        }
    }
    std::cout << "closing file" << std::endl;
    fclose(outputFile);
   
    return ;
  }
        
}

 