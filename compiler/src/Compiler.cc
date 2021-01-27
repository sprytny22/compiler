#include <iostream>
#include <sstream>
#include <string>
#include "../headers/Compiler.h"
#include "../headers/Debug.h"

Compiler::Compiler() {
    _stack = new std::stack<Variable*>();
    _symbols = new std::map<std::string, Variable*>();
    _assembly = new Assembly();
}

void Compiler::pushOnStack(Variable* variable) {
    _stack->push(variable);
}

void Compiler::createThree(std::string op) {

    if (_stack->size() < 1) {
        return;
    }

    LexType resultType = LexType::Int;

    Variable* right = topAndPop();
    Variable* left = topAndPop();

    std::string rightValue = right->getValue();
    std::string leftValue = left->getValue();

    if (isTextValue(rightValue)) {
        auto symbol = findSymbol(right->getValue());
        if (symbol == nullptr) {
            Debug::info("Symbol not exists right! (createThree)");
            return;
        }

        _assembly->lw("$t0", right->getValue());
    }
    else {
        _assembly->li("$t0", right->getValue());
    }

    if (isTextValue(leftValue)) {
        auto symbol = findSymbol(left->getValue());
        if (symbol == nullptr) {
            Debug::info("Symbol not exists left! (createThree)");
            return;
        }

        _assembly->lw("$t1", left->getValue());
    }
    else {
        _assembly->li("$t1", left->getValue());
    }

    if (right->getLexType() == LexType::Double || left->getLexType() == LexType::Double) {
        resultType = LexType::Double;
    }


    std::string unique = Variable::generateUniqueName();
    Variable* result = new Variable(resultType, unique);
    pushOnStack(result);

    _assembly->action(op, "$t0", "$t0", "$t1");
    _assembly->sw("$t0", result->getValue());

    createSymbol(unique, result);
    _assembly->data(result->getValue(),result->getLexType());
}

void Compiler::createSymbol(std::string name, Variable* variable) {
    if (findSymbol(name) != nullptr) {
        Debug::info("Variable is already defined!");
        return;
    }

    _symbols->insert(std::make_pair(name, variable));
}

void Compiler::simpleAssigmentInt(std::string variableName) {
    Variable* top = topAndPop();
    LexType type = top->getLexType();
    std::string value = top->getValue();

    if (type == LexType::Double) {
        Debug::info("Cant assigne double to int!");
        return;
    }

    if (isTextValue(value)) {
        if (!correctSymbol(top)) {
            Debug::info("symbol not exists! (simpleAssigmentInt)");
            return;
        }
    }
    createSymbol(variableName, top);

    assigmentAssembly(variableName, top);
}

void Compiler::simpleAssigmentDouble(std::string variableName) {
    Variable* top = topAndPop();
    LexType type = top->getLexType();
    std::string value = top->getValue();

    if (type == LexType::Int) {
        top->setLexType(LexType::Double); //convert int to double
        return;
    }

    if (isTextValue(value)) {
        top->setLexType(LexType::Double);

        if (!correctSymbol(top)) {
            Debug::info("symbol not exists! (simpleAssigmentDouble)");
            return;
        }
    }

    createSymbol(variableName, top);
    assigmentAssembly(variableName, top);
}

void Compiler::simpleAssigmentText(std::string variableName) {
    Variable* top = topAndPop();

    if (!correctSymbol(top)) {
        Debug::info("symbol not declarated!");
        return;
    }

    assigmentAssembly(variableName, top);
}

bool Compiler::correctSymbol(Variable* variable) {
    LexType type;
    std::string value = variable->getValue();
    Variable* symbol = nullptr;

    if ((symbol = findSymbol(value)) == nullptr) {
        Debug::info("symbol not exists!");
        return false;
    }

    type = symbol->getLexType();// TODO: type int -> double, double -> int is CORRECT!
    if (type != variable->getLexType()) {
        Debug::info("symbol not match!");
        return false;
    }

    return true;
} 

Variable* Compiler::findSymbol(std::string value) {
    Variable* type = nullptr;
    auto found = _symbols->find(value);
    
    if (found != _symbols->end()) {
        type = found->second;
    }

    return type;
}

LexType Compiler::findSymbolType(std::string value) {
    return findSymbol(value)->getLexType();
}

Variable* Compiler::topAndPop(){
    Variable* top = _stack->top();
    _stack->pop();

    return top;
}

bool Compiler::isTextValue(std::string value) {
  std::stringstream ss(value); 
  int number;
  float fnumber;

  Debug::info("value: " + value);

  if((ss >> number).fail() && (ss >> fnumber).fail())
  { 
      return true;
  }

  return false;
}

void Compiler::assigmentAssembly(std::string variableName, Variable* top) {
    isTextValue(top->getValue()) ? _assembly->lw("$t0", top->getValue()) : _assembly->li("$t0", top->getValue());

    _assembly->data(variableName, top->getLexType());    
    _assembly->sw("$t0", variableName);
    _assembly->generateOutputFile();
}