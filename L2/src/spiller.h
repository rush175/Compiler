#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <iterator>
#include <regex>
#include <stdlib.h>
#include <algorithm>

#define DEBUGGING 0



namespace L2{

	L2::Variable* findVarInFunction(std::string name, Function* f){
        for(Variable* var : f->interferenceGraph->variables){
        	if(DEBUGGING) printf("Comparing %s and %s\n", name.c_str(), var->name.c_str());
            if(name == var->name){
                return var;
            }
        }
        if(DEBUGGING) printf("Could not find the var %s\n", name.c_str());
        return NULL;
    }

	void generateUsesAndVars(L2::Function* f){
		if(DEBUGGING) printf("Beginning generateInterferenceGraph\n");
        L2::InterferenceGraph* iG = new L2::InterferenceGraph();
        f->interferenceGraph = iG;
        //iG->variables = {};
        if(DEBUGGING) printf("instatiateVariables Time\n");
        instatiateVariables(f, iG);
		for(Instruction* I : f->instructions){
			for(int i = 0; i < I->arguments.size(); i++){
				L2::Variable* V = findCorrespondingVar(I->arguments[i]->name, iG);
				//Found a variable
				if(V){
					//varFound = true;
					V->uses.push_back(I);
					if(I->type == ASSIGN){ 
						if(I->arguments[0]->name == I->arguments[1]->name){
							break;
						}
					}
				}
			}
		}
	}
	
	void generateInstNums(L2::Function* f){
		int i = 0;
		linkInstructionPointers(f);
		for(Instruction* I : f->instructions){
			I->instNum = i;
			i++;
			//Handling inc/dec because we were doing bad programming before
		}

	}

	void removeIncDecSpaces(L2::Function* f){
		for(Instruction* I : f->instructions){
			if(I->type == INC_DEC){
				I->instruction.erase(remove(I->instruction.begin(), I->instruction.end(), ' '), I->instruction.end());
			}
		}
	}


	void printNewSpill(Function* f){

		printf("(%s\n", f->name.c_str());
		printf("\t%ld %ld\n", f->arguments, f->locals);

		for (Instruction* I : f->instructions) {
			printf("\t%s\n", I->instruction.c_str());
		}

		printf(")\n");
	}


	void insertLoad(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc) {
		Instruction* newInst = new Instruction();
		//Load inst
		newInst->type = LOAD;
		newInst->instruction = replacementString + " <- "+ "mem rsp " + std::to_string(stackLoc);

		L2::Arg* arg = new L2::Arg();
		arg->name = replacementString;
		arg->type = MEM;

		L2::Arg* arg2 = new L2::Arg();
		arg2->name = "mem rsp " + std::to_string(stackLoc);
		arg2->type = MEM;

		newInst->arguments.push_back(arg);
		newInst->arguments.push_back(arg2);
		newInst->operation.push_back("<-");

		f->instructions.insert(idx, newInst);
	}

	void insertStore(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc) {
		Instruction* newInst = new Instruction();
		//Store inst
		newInst->instruction = "mem rsp " + std::to_string(stackLoc) + " <- "+ replacementString;
		newInst->type = STORE;

		L2::Arg* arg = new L2::Arg();
		arg->name = "mem rsp " + std::to_string(stackLoc);
		arg->type = MEM;

		L2::Arg* arg2 = new L2::Arg();
		arg2->name = replacementString;
		arg2->type = MEM;

		newInst->arguments.push_back(arg);
		newInst->arguments.push_back(arg2);
		newInst->operation.push_back("<-");

		f->instructions.insert(idx, newInst);
	}


	void spillVar(L2::Function* f){
		
		removeIncDecSpaces(f);
		generateUsesAndVars(f);
		generateInstNums(f);
		
		Variable* V = findVarInFunction(f->toSpill, f);
		
		int i = 0;
		if(V){ 
			f->locals++;
			int stackLoc = (f->locals * 8) - 8;
			int numUses = V->uses.size();

		
			while(V->uses.size() > 0){

				std::vector<Instruction*>::iterator iter = V->uses.begin();
				Instruction* I = *iter;

				int j = 0;
				std::string replacementString = f->replaceSpill + std::to_string(i);

				for(Arg* curArg : I->arguments){
					if(curArg->name == f->toSpill){
						I->arguments[j]->name = replacementString;
					}
					
					I->instruction = std::regex_replace(I->instruction, std::regex(f->toSpill), replacementString);
					
					j++;
				}

				std::vector<Instruction*>::iterator iter2;
				iter2 = f->instructions.begin();
				
				for(std::string g : I->gen){
					if(g == V->name){
						insertLoad(f, replacementString, iter2 + I->instNum, stackLoc);
						generateInstNums(f);
						iter2 = f->instructions.begin();
						break;
					}
				}
				iter2 = f->instructions.begin();
				if(I->type == LOAD || I->type == ASSIGN || I->type == AOP || I->type == INC_DEC || I->type == CMP_ASSIGN || I->type == LEA){
					if(I->kill[0] == V->name){
						insertStore(f, replacementString, iter2 + I->instNum + 1, stackLoc);
						generateInstNums(f);
						iter2 = f->instructions.begin();
					}
				}


				
				int k = 0;
				std::vector<Instruction*> newUses = {};
				for(Instruction* I : V->uses){
					if (k) {   newUses.push_back(I);   }
					else   {   k++;				 	   }
				}
				
				V->uses = newUses;

				generateInstNums(f);
				i++;
			}
		}
		else{
			if(DEBUGGING) printf("Not a valid VAR\n");
		}

	}


}
