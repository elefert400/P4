#include "ast.hpp"
#include "symbol_table.hpp"
#include "err.hpp"
#include "types.hpp"

namespace lake{

void ProgramNode::typeAnalysis(TypeAnalysis * ta){

	//pass the TypeAnalysis down throughout
	// the entire tree, getting the types for
	// each element in turn and adding them
	// to the ta object's hashMap
	this->myDeclList->typeAnalysis(ta);

	//The type of the program node will never
	// be needed. We can just set it to VOID
	ta->nodeType(this, VarType::produce(VOID));

	//Alternatively, we could make our type
	// be error if the DeclListNode is an error

	//Lookup the type assigned to the declList
	// in the earlier recursive call
	const DataType * childType =
		ta->nodeType(myDeclList);

	//The asError() function of the DataType
	// class returns null (false) in every
	// subclass EXCEPT for the ErrorType subclass,
	// where it returns itself (non-null/true).
	if (childType->asError()){
		//The child type is error, so
		// set the program node to error
		// as well
		ta->nodeType(this, ErrorType::produce());
	}
}

void DeclListNode::typeAnalysis(TypeAnalysis * ta){
	ta->nodeType(this, VarType::produce(VOID));

	for (auto decl : *myDecls){
		//Do typeAnalysis on the single decl
		decl->typeAnalysis(ta);
		//Lookup the type that we added
		// to the ta in the recursive call
		// above
		auto eltType = ta->nodeType(decl);
		//If the element type was the special
		// "error" type, set this node to the errorType
		if (eltType->asError()){
			ta->nodeType(this, ErrorType::produce());
		}
	}
	return;
}

void FnDeclNode::typeAnalysis(TypeAnalysis * ta){

	//HINT: you might want to change the signature for
	// typeAnalysis on FnBodyNode to take a second
	// argument which is the type of the current
	// function. This will help you to know at a
	// return statement whether the return type matches
	// the current function

	//Note, this function may need extra code
	myBody->typeAnalysis(ta, myRetAST);
	const DataType* myBodyType = ta->nodeType(myBody);
	if(myBodyType->asError()){
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void FnBodyNode::typeAnalysis(TypeAnalysis * ta, TypeNode* fnRetType){
	//HINT: as above, you may want to pass the
	// fnDecl's type into the statement list as a
	// second argument to StmtList::typeAnalysis

	//Note, this function may need extra code
	myStmtList->typeAnalysis(ta, fnRetType);
	const DataType* myStmtListType = ta->nodeType(myStmtList);
	if(myStmtListType->asError()){
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void StmtListNode::typeAnalysis(TypeAnalysis * ta, TypeNode* fnRetType){
	//Note, this function may need extra code
	bool valid = true;
	const DataType* myType;
	for (auto stmt : *myStmts){

		stmt->typeAnalysis(ta, fnRetType);
		myType = ta->nodeType(stmt);
		if(myType->asError())
		{
			valid = false;
		}
	}
	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}

void StmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode* fnRetType){
	TODO("Implement me in the subclass");
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * ta, TypeNode* fnRetType){

	myAssign->typeAnalysis(ta);

	//It can be a bit of a pain to write
	// "const DataType *" everywhere, so here
	// the use of auto is used instead to tell the
	// compiler to figure out what the subType variable
	// should be

	//const DataType*
	auto subType = ta->nodeType(myAssign);

	if (subType->asError()){
		ta->nodeType(this, subType);
	} else {
		ta->nodeType(this, VarType::produce(VOID));
	}
}

void PostIncStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	const DataType * ExpType = ta->nodeType(myExp);

	bool valid = true;

	//checking if myexp is an error
	if(ExpType->asError()){
		valid = false;
	}
	//checking if exp is of type int or ptr
	if(!(ExpType->isInt() || ExpType->isPtr()))
	{
		ta->badMathOpr(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		ta->nodeType(this, ExpType);
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void PostDecStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	const DataType * ExpType = ta->nodeType(myExp);

	bool valid = true;

	//checking if myexp is an error
	if(ExpType->asError()){
		valid = false;
	}
	//checking if exp is of type int or ptr
	if(!(ExpType->isInt() || ExpType->isPtr()))
	{
		ta->badMathOpd(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		ta->nodeType(this, ExpType);
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void ReadStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);

	bool valid = true;

	if(expType->asError())
	{
			valid = false;
	}

	if(expType->asFn())
	{
		ta->readFn(this->getLine(), this->getCol());
		valid = false;
	}
	if(expType->isPtr())
	{
		ta->badReadPtr(this->getLine(), this->getCol());
		valid = false;
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void WriteStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	const DataType * expType = ta->nodeType(myExp);

	bool valid = true;

	if(expType->asError())
	{
		valid = false;
	}

	if(expType->asFn())
	{
		ta->writeFn(this->getLine(), this->getCol());
		valid = false;
	}
	if(expType->isVoid())
	{
		ta->badWriteVoid(this->getLine(), this->getCol());
		valid = false;
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void IfStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	myStmts->typeAnalysis(ta, fnRetType);
	const DataType * expType = ta->nodeType(myExp);
	const DataType * stmtType = ta->nodeType(myStmts);

	bool valid = true;

	if(expType->asError() || stmtType->asError())
	{
		valid = false;
	}

	if(!(expType->isBool()))
	{
		ta->badIfCond(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void IfElseStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	myStmtsT->typeAnalysis(ta, fnRetType);
	myStmtsF->typeAnalysis(ta, fnRetType);
	const DataType * expType = ta->nodeType(myExp);
	const DataType * stmtTypeT = ta->nodeType(myStmtsT);
	const DataType * stmtTypeF = ta->nodeType(myStmtsF);

	bool valid = true;

	if(expType->asError() || stmtTypeT->asError() || stmtTypeF->asError())
	{
		valid = false;
	}

	if(!(expType->isBool()))
	{
		ta->badIfCond(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void WhileStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myExp->typeAnalysis(ta);
	myStmts->typeAnalysis(ta, fnRetType);
	const DataType * expType = ta->nodeType(myExp);
	const DataType * stmtType = ta->nodeType(myStmts);

	bool valid = true;

	if(expType->asError() || stmtType->asError())
	{
		valid = false;
	}

	if(!(expType->isBool()))
	{
		ta->badWhileCond(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(VOID));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void ReturnStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	bool valid = true;
	// if(myExp == nullptr && fnRetType->getDataType()->isVoid()){
	// 	//valid
	// }
	if(myExp == nullptr && !fnRetType->getDataType()->isVoid()){
		ta->badNoRet(this->getLine(), this->getCol());
		valid = false;
	}
	else if(myExp != nullptr){
		myExp->typeAnalysis(ta);
		const DataType * expType = ta->nodeType(myExp);
		if(!expType->asError()){
			//check if we are in void fn
			if(fnRetType->getDataType()->isVoid()){
				//both are void
				if(!expType->isVoid()){
					ta->extraRetValue(myExp->getLine(), myExp->getCol());
					valid = false;
				}
			}
			//non-void fn
			else{
				//match types, return error if they do not match
				if((expType->isInt() && fnRetType->getDataType()->isInt()) ||
					(expType->isBool() && fnRetType->getDataType()->isBool())){
					//Pointer: int OR bool
					if(expType->isPtr() && fnRetType->getDataType()->isPtr()){
						//check mismatched pointer depth
						if(static_cast<const VarType*>(fnRetType->getDataType())->getDepth()
										!= static_cast<const VarType*>(expType)->getDepth()){
							ta->badRetValue(myExp->getLine(), myExp->getCol());
							valid = false;
						}
						//else fell through to here, pointer depths match
					}
					// else{
					// 	ta->badRetValue(myExp->getLine(), myExp->getCol());
					// 	valid = false;
					// }
					//else fell through to here, matched types successfully
				}
				else{
					ta->badRetValue(myExp->getLine(), myExp->getCol());
					valid = false;
				}
			}
		}
	}
	if(!valid){
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, VarType::produce(VOID));
	}
}
void ExpListNode::typeAnalysis(TypeAnalysis* ta){
	std::list<const DataType*> * argsList = new std::list<const DataType*>();
	for(std::list<ExpNode*>::iterator it=myExps->begin(); it != myExps->end(); ++it){

		(*it)->typeAnalysis(ta);
		argsList->push_back(ta->nodeType(*it));
	}
	TupleType* myTuple = new TupleType(argsList);
	ta->nodeType(this, myTuple);
}
void CallStmtNode::typeAnalysis(TypeAnalysis* ta, TypeNode* fnRetType){
	myCallExp->typeAnalysis(ta);
	const DataType* callType = ta->nodeType(myCallExp);
	bool valid = true;
	if(callType->asError()){
		valid = false;
	}
	//not a function so report an error
	else if(callType->asVar() == nullptr){
		ta->badCallee(this->getLine(), this->getCol());
		valid = false;
	}
	if(!valid){
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, VarType::produce(VOID));
	}
}
void CallExpNode::typeAnalysis(TypeAnalysis* ta){
	bool valid = true;
	myId->typeAnalysis(ta);
	myExpList->typeAnalysis(ta);
	if(ta->nodeType(myId)->asFn() == nullptr){
		ta->badCallee(myId->getLine(), myId->getCol());
		valid = false;
	}
	else{
		const FnType* idFnType = static_cast<const FnType*>(ta->nodeType(myId));
		const TupleType*  argsType = static_cast<const TupleType*>(ta->nodeType(myExpList));
		if(idFnType->asError() || argsType->asError()){

			valid = false;
		}
		//const TupleType * idArgsType = idFnType->getFormalTypes();

		std::list<const DataType *> * listExpectedArgs = idFnType->getFormalTypes()->getElts();
		std::list<const DataType *> * listGivenArgs = argsType->getElts();
		if(listExpectedArgs->size() != listGivenArgs->size()){
			ta->badArgCount(myId->getLine(), myId->getCol());
			valid = false;
		}
		std::list<const DataType *>::iterator l1 = listExpectedArgs->begin();
		std::list<const DataType *>::iterator l2 = listGivenArgs->begin();
		std::list<ExpNode*> * argsAsExpNodes = myExpList->getList();
		/*getting the actual list from myExpList to iterate over simult.
		Should be same size as listGivenArgs.*/
		std::list<ExpNode*>::iterator l_Extra = myExpList->getList()->begin();

		while(l1 != listExpectedArgs->end() && l2 != listGivenArgs->end() && l_Extra != argsAsExpNodes->end()){
			if((*l1)->getString() != (*l2)->getString()){
				if(!(*l2)->asError()){
					ta->badArgMatch((*l_Extra)->getLine(),(*l_Extra)->getCol());
				}
				// ta->badArgMatch(myId->getLine(),(myId->getCol() + myId->getSymbol()->getName().length() + 1));
				valid = false;
			}
			++l1;
			++l2;
			++l_Extra;
		}
	}


	if(!valid){
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, static_cast<const FnType*>(ta->nodeType(myId))->getReturnType());
	}

}
void PlusNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;
	// if(Exp1Type->isBool()){
	// 	valid = false;
	// }
	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
		valid = false;
	}
	//checking RHS is valid
	if(!(Exp1Type->isInt() || Exp1Type->isPtr()))
	{
		ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
		valid = false;
	}
	//checking LHS is valid
	if(!(Exp2Type->isInt() || Exp2Type->isPtr()))
	{
		ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		//int + int
		if(Exp1Type->isInt() && Exp2Type->isInt()){
			ta->nodeType(this, Exp2Type);
		}
		//int + ptr
		else if(Exp1Type->isInt() && Exp2Type->isPtr()){
			ta->nodeType(this, Exp2Type);
		}
		//ptr + int
		else if(Exp2Type->isInt() && Exp1Type->isPtr()){
			ta->nodeType(this, Exp1Type);
		}
		//types are not incompatible
		else
		{
			ta->badMathOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void MinusNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
		valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt() || Exp1Type->isPtr()))
	{
		ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
		valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt() || Exp2Type->isPtr()))
	{
		ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		//int + int
		if(Exp1Type->isInt() && Exp2Type->isInt()){
			ta->nodeType(this, Exp2Type);
		}
		//int + ptr
		else if(Exp1Type->isInt() && Exp2Type->isPtr()){
			ta->nodeType(this, Exp2Type);
		}
		//ptr + int
		else if(Exp2Type->isInt() && Exp1Type->isPtr()){
			ta->nodeType(this, Exp1Type);
		}
		//types are not incompatible
		else
		{
			ta->badMathOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void TimesNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
		valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt() || Exp1Type->isPtr()))
	{
		ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
		valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt() || Exp2Type->isPtr()))
	{
		ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		//int + int
		if(Exp1Type->isInt() && Exp2Type->isInt()){
			ta->nodeType(this, Exp2Type);
		}
		//int + ptr
		else if(Exp1Type->isInt() && Exp2Type->isPtr()){
			ta->nodeType(this, Exp2Type);
		}
		//ptr + int
		else if(Exp2Type->isInt() && Exp1Type->isPtr()){
			ta->nodeType(this, Exp1Type);
		}
		//types are not incompatible
		else
		{
			ta->badMathOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void DivideNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
		valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt() || Exp1Type->isPtr()))
	{
		ta->badMathOpd(myExp1->getLine(), myExp1->getCol());
		valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt() || Exp2Type->isPtr()))
	{
		ta->badMathOpd(myExp2->getLine(), myExp2->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		//int + int
		if(Exp1Type->isInt() && Exp2Type->isInt()){
			ta->nodeType(this, Exp2Type);
		}
		//int + ptr
		else if(Exp1Type->isInt() && Exp2Type->isPtr()){
			ta->nodeType(this, Exp2Type);
		}
		//ptr + int
		else if(Exp2Type->isInt() && Exp1Type->isPtr()){
			ta->nodeType(this, Exp1Type);
		}
		//types are not incompatible
		else
		{
			ta->badMathOpr(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void AndNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isBool())){
		ta->badLogicOpd(myExp1->getLine(), myExp1->getCol());
	  valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isBool())){
		ta->badLogicOpd(myExp2->getLine(), myExp2->getCol());
	  valid = false;
	}

	//inserting valids into the HashMap
	if(valid)
	{	//bool && bool
	    ta->nodeType(this, Exp2Type);
	}

	//not valid cases
	else{
		ta->nodeType(this, ErrorType::produce());
	}
}

void OrNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isBool())){
		ta->badLogicOpd(myExp1->getLine(), myExp1->getCol());
	  valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isBool())){
		ta->badLogicOpd(myExp2->getLine(), myExp2->getCol());
	  valid = false;
	}

	//inserting valids into the HashMap
	if(valid)
	{	//bool && bool
	    ta->nodeType(this, Exp2Type);
	}

	//not valid cases
	else{
		ta->nodeType(this, ErrorType::produce());
	}
}
void EqualsNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are the same type,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(Exp1Type->asVar() == nullptr ||
		Exp2Type->asVar() == nullptr ||
		Exp1Type->isVoid() ||
		Exp2Type->isVoid()){
			ta->badEqOpd(myExp1->getLine(), myExp1->getCol());
			valid = false;
		}
	//checking LHS is valid
	else if(Exp1Type->isBool() != Exp2Type->isBool()){
		ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
	  	valid = false;
	}
	else if(Exp1Type->isInt() != Exp2Type->isInt()){
		ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
	  	valid = false;
	}
	else if(Exp1Type->isPtr() == Exp2Type->isPtr()){
		if(Exp1Type->asVar()->getDepth() != Exp2Type->asVar()->getDepth()){
			ta->badEqOpr(this->getLine(), this->getCol());
			valid = false;
		}
	}
	//inserting valids into the HashMap
	if(valid)
	{	//bool && bool
	    ta->nodeType(this, VarType::produce(BOOL));
	}

	//not valid cases
	else{
		ta->nodeType(this, ErrorType::produce());
	}
}
void NotEqualsNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are the same type,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;
	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(Exp1Type->asVar() == nullptr ||
		Exp2Type->asVar() == nullptr ||
		Exp1Type->isVoid() ||
		Exp2Type->isVoid()){
			ta->badEqOpd(this->getLine(), this->getCol());
			valid = false;
		}

	//checking LHS is valid
	else if(Exp1Type->isBool() != Exp2Type->isBool()){

		ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
	  	valid = false;
	}
	else if(Exp1Type->isInt() != Exp2Type->isInt()){
		ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
	  	valid = false;
	}
	else if(Exp1Type->isPtr() == Exp2Type->isPtr()){
		if(Exp1Type->asVar()->getDepth() != Exp2Type->asVar()->getDepth()){
			ta->badEqOpr(myExp1->getLine(), myExp1->getCol());
			valid = false;
		}
	}
	//inserting valids into the HashMap
	if(valid)
	{	//bool && bool
	   ta->nodeType(this, VarType::produce(BOOL));
	}

	//not valid cases
	else{
		ta->nodeType(this, ErrorType::produce());
	}
}
void LessNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt()))
	{
		ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
	  valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt()))
	{
		ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
	  valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
	  //int + int
	  if(Exp1Type->isInt() && Exp2Type->isInt())
		{
	    ta->nodeType(this, VarType::produce(BOOL));
	  }
	  //types are not incompatible
	  else
	  {
	    ta->badRelOpd(this->getLine(), this->getCol());
	    ta->nodeType(this, ErrorType::produce());
	  }
	}

	//not valid cases
	else
	{
	  ta->nodeType(this, ErrorType::produce());
	}
}
void GreaterNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt()))
	{
		ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
	  valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt()))
	{
		ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
	  valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
	  //int + int
	  if(Exp1Type->isInt() && Exp2Type->isInt())
		{
	    ta->nodeType(this, VarType::produce(BOOL));
	  }
	  //types are not incompatible
	  else
	  {
	    ta->badRelOpd(this->getLine(), this->getCol());
	    ta->nodeType(this, ErrorType::produce());
	  }
	}

	//not valid cases
	else
	{
	  ta->nodeType(this, ErrorType::produce());
	}
}
void LessEqNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
		valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt()))
	{
		ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
		valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt()))
	{
		ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		//int + int
		if(Exp1Type->isInt() && Exp2Type->isInt())
		{
			ta->nodeType(this, VarType::produce(BOOL));
		}
		//types are not incompatible
		else
		{
			ta->badRelOpd(this->getLine(), this->getCol());
			ta->nodeType(this, ErrorType::produce());
		}
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void GreaterEqNode::typeAnalysis(TypeAnalysis* ta){
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);
	const DataType * Exp1Type = ta->nodeType(myExp1);
	const DataType * Exp2Type = ta->nodeType(myExp2);
	/*
	check if they are functions, check if they are ints,
	check if the ptr depth is the same,
	if all pass pass the typeAnalysis and add its type to the hashMap
	in ta
	*/
	bool valid = true;

	//checking if either is an error
	if(Exp1Type->asError() || Exp2Type->asError()){
	  valid = false;
	}

	//checking RHS is valid
	if(!(Exp1Type->isInt()))
	{
		ta->badRelOpd(myExp1->getLine(), myExp1->getCol());
	  valid = false;
	}

	//checking LHS is valid
	if(!(Exp2Type->isInt()))
	{
		ta->badRelOpd(myExp2->getLine(), myExp2->getCol());
	  valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
	  //int + int
	  if(Exp1Type->isInt() && Exp2Type->isInt())
		{
	    ta->nodeType(this, VarType::produce(BOOL));
	  }
	  //types are not incompatible
	  else
	  {
	    ta->badRelOpd(this->getLine(), this->getCol());
	    ta->nodeType(this, ErrorType::produce());
	  }
	}

	//not valid cases
	else
	{
	  ta->nodeType(this, ErrorType::produce());
	}
}
//negative numbers
void UnaryMinusNode::typeAnalysis(TypeAnalysis* ta){
	myExp->typeAnalysis(ta);
	const DataType * ExpType = ta->nodeType(myExp);

	bool valid = true;
	if(ExpType->asError())
	{
		valid = false;
	}

	if(!(ExpType->isInt()))
	{
		valid = false;
		ta->badMathOpr(this->getLine(), this->getCol());
	}

	if(valid)
	{
		ta->nodeType(this, VarType::produce(INT));
	}
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void NotNode::typeAnalysis(TypeAnalysis* ta){
	myExp->typeAnalysis(ta);
	const DataType * ExpType = ta->nodeType(myExp);

	bool valid = true;

	//checking if myexp is an error
	if(ExpType->asError()){
		valid = false;
	}
	//checking if exp is of type int or ptr
	if(!(ExpType->isBool()))
	{
		ta->badLogicOpd(myExp->getLine(), myExp->getCol());
		valid = false;
	}

	//inserting valids into hashMap
	if(valid)
	{
		ta->nodeType(this, ExpType);
	}

	//not valid cases
	else
	{
		ta->nodeType(this, ErrorType::produce());
	}
}
void ExpNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void AssignNode::typeAnalysis(TypeAnalysis * ta){
	//TODO: Note that this function is incomplete.
	// and needs additional code

	//Do typeAnalysis on the subexpressions
	myTgt->typeAnalysis(ta);
	mySrc->typeAnalysis(ta);

	const DataType * tgtType = ta->nodeType(myTgt);
	const DataType * srcType = ta->nodeType(mySrc);
	bool valid = true;
	if(tgtType->asError() || srcType->asError()){
		// ta->nodeType(this, ErrorType::produce());
		valid = false;
	}
	//While incomplete, this gives you one case for
	// assignment: if the types are exactly the same
	// it is usually ok to do the assignment. One
	// exception is that if both types are function
	// names, it should fail type analysis
	else if(tgtType->asFn() != nullptr || srcType->asFn() != nullptr){
		ta->badAssignOpd(this->getLine(), this->getCol());
		//ta->nodeType(this, ErrorType::produce());
		valid = false;
	}
	else if(srcType->asFn() != nullptr){
		ta->badAssignOpd(mySrc->getLine(), myTgt->getCol());
		//ta->nodeType(this, ErrorType::produce());
		valid = false;
	}
	if(!valid){
		//Some functions are already defined for you to
		// report type errors. Note that these functions
		// also tell the typeAnalysis object that the
		// analysis has failed, meaning that main.cpp
		// will print "Type check failed" at the end
		//ta->badAssignOpr(this->getLine(), this->getCol());


		//Note that reporting an error does not set the
		// type of the current node, so setting the node
		// type must be done
		ta->nodeType(this, ErrorType::produce());
	}
	else if (tgtType == srcType){
		ta->nodeType(this, tgtType);
	}
}

void DeclNode::typeAnalysis(TypeAnalysis * ta){
	TODO("Override me in the subclass");
}

void VarDeclNode::typeAnalysis(TypeAnalysis * ta){
	// VarDecls always pass type analysis, since they
	// are never used in an expression. You may choose
	// to type them void (like this).
	ta->nodeType(this, VarType::produce(VOID));

	//Alternatively, you could give the VarDecl
	// the type of the symbol it declares (this works
	// because it's type was attached during
	// nameAnalysis)
	ta->nodeType(this, myID->getSymbol()->getType());
}

void IdNode::typeAnalysis(TypeAnalysis * ta){
	// IDs never fail type analysis and always
	// yield the type of their symbol (which
	// depends on their definition)
	ta->nodeType(this, this->getSymbol()->getType());
	// const DataType * tgtType = ta->nodeType(this);
	// std::cout << tgtType->isPtr() << "\n";
}
void DerefNode::typeAnalysis(TypeAnalysis * ta){
	myTgt->typeAnalysis(ta);
	bool valid = true;
	const DataType * tgtType = ta->nodeType(myTgt);
	const DataType * tgtCheck;
	if(tgtType->asError())
	{
		valid = false;
		// ta->nodeType(this, ErrorType::produce());
		// return;
	}
	else{
		tgtCheck = tgtType->asVar()->getDerefType();
	}
	if(tgtCheck == nullptr){
		ta->badDeref(this->getLine(), this->getCol());
		valid = false;
	}
	// else if(tgtCheck->asVar()->getDepth() >= 0){
	// 	ta->nodeType(this, tgtCheck);
	// }
	if(!valid){
		
		ta->nodeType(this, ErrorType::produce());
	}
	else{
		ta->nodeType(this, tgtCheck);
	}
}
void IntLitNode::typeAnalysis(TypeAnalysis * ta){
	// IntLits never fail their type analysis and always
	// yield the type INT
	ta->nodeType(this, VarType::produce(INT));
}
void TrueNode::typeAnalysis(TypeAnalysis * ta) {
	ta->nodeType(this, VarType::produce(BOOL));
}
void FalseNode::typeAnalysis(TypeAnalysis * ta) {
	ta->nodeType(this, VarType::produce(BOOL));
}

}//end namespace Lake
