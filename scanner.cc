#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <utility>
#include <set>
#include <array>
#include <iostream>
#include "scanner.h"

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains helpers for asm.cc and you don't need to modify it.
 * Furthermore, while this code may be helpful to understand starting with
 * the DFA assignments, you do not need to understand it to write the assembler.
 */

Token::Token(Token::Kind kind, std::string lexeme):
  kind(kind), lexeme(std::move(lexeme)) {}

  Token:: Kind Token::getKind() const { return kind; }
const std::string &Token::getLexeme() const { return lexeme; }

std::ostream &operator<<(std::ostream &out, const Token &tok) {
  switch (tok.getKind()) {
    case Token::ID:         out << "ID";         break;
    case Token::NUM:        out << "NUM";        break;
    case Token::LPAREN:     out << "LPAREN";     break;
    case Token::RPAREN:     out << "RPAREN";     break;
    case Token::LBRACE:     out << "LBRACE";     break;
    case Token::RBRACE:     out << "RBRACE";     break;
    case Token::RETURN:     out << "RETURN";     break;
    case Token::IF:         out << "IF";         break;
    case Token::ELSE:       out << "ELSE";       break;
    case Token::WHILE:      out << "WHILE";      break;
    case Token::PRINTLN:    out << "PRINTLN";    break;
    case Token::WAIN:       out << "WAIN";       break;
    case Token::BECOMES:    out << "BECOMES";    break;
    case Token::INT:        out << "INT";        break;
    case Token::EQ:         out << "EQ";         break;
    case Token::NE:         out << "NE";         break;
    case Token::LT:         out << "LT";         break;
    case Token::GT:         out << "GT";         break;
    case Token::LE:         out << "LE";         break;
    case Token::GE:         out << "GE";         break;
    case Token::PLUS:       out << "PLUS";       break;
    case Token::MINUS:      out << "MINUS";      break;
    case Token::STAR:       out << "STAR";       break;
    case Token::SLASH:      out << "SLASH";      break;
    case Token::PCT:        out << "PCT";        break;
    case Token::COMMA:      out << "COMMA";      break;
    case Token::SEMI:       out << "SEMI";       break;
    case Token::NEW:        out << "NEW";        break;
    case Token::DELETE:     out << "DELETE";     break;
    case Token::LBRACK:     out << "LBREAK";     break;
    case Token::RBRACK:     out << "RBREAK";     break;
    case Token::AMP:        out << "AMP";        break;
    case Token::NULL_TYPE:  out << "NULL";       break;
    case Token::WHITESPACE: out << "WHITESPACE"; break;
    case Token::COMMENT:    out << "COMMENT";    break;
  }
  out << " " << tok.getLexeme();

  return out;
}

int64_t Token::toLong() const {
  std::istringstream iss;
  int64_t result;

  if (kind == NUM) {
    iss.str(lexeme);
  } else {
    // This should never happen if the user calls this function correctly
    return 0;
  }

  iss >> result;
  return result;
}

ScanningFailure::ScanningFailure(std::string message):
  message(std::move(message)) {}

const std::string &ScanningFailure::what() const { return message; }

/* Represents a DFA (which you will see formally in class later)
 * to handle the scanning
 * process. You should not need to interact with this directly:
 * it is handled through the starter code.
 */
class wlp4DFA {
  public:
    enum State {
      // States that are also kinds
      ID = 0,
      NUM,
      LPAREN,
      RPAREN,
      LBRACE,
      RBRACE,
      RETURN,
      IF,
      ELSE,
      WHILE,
      PRINTLN,
      WAIN,
      BECOMES,
      INT,
      EQ,
      NE,
      LT,
      GT,
      LE,
      GE,
      PLUS,
      MINUS,
      STAR,
      SLASH,
      PCT,
      COMMA,
      SEMI,
      NEW,
      DELETE,
      LBRACK,
      RBRACK,
      AMP,
      NULL_TYPE,
      WHITESPACE,
      COMMENT,

      // States that are not also kinds
      FAIL,
      START,
      GanTanHao,
      ZERO,

      // Hack to let this be used easily in arrays. This should always be the
      // final element in the enum, and should always point to the previous
      // element.

      LARGEST_STATE = ZERO
    };

  private:
    /* A set of all accepting states for the DFA.
     * Currently non-accepting states are not actually present anywhere
     * in memory, but a list can be found in the constructor.
     */
    std::set<State> acceptingStates;

    /*
     * The transition function for the DFA, stored as a map.
     */

    std::array<std::array<State, 128>, LARGEST_STATE + 1> transitionFunction;

    /*
     * Converts a state to a kind to allow construction of Tokens from States.
     * Throws an exception if conversion is not possible.
     */
    Token::Kind stateToKind(State s) const {
      switch(s) {
        case ID:         return Token::ID;
        case NUM:        return Token::NUM;
        case LPAREN:     return Token::LPAREN;
        case RPAREN:     return Token::RPAREN;
        case LBRACE:     return Token::LBRACE;
        case RBRACE:     return Token::RBRACE;
        case RETURN:     return Token::RETURN;
        case IF:         return Token::IF;
        case ELSE:       return Token::ELSE;
        case WHILE:      return Token::WHILE;
        case PRINTLN:    return Token::PRINTLN;
        case WAIN:       return Token::WAIN;
        case BECOMES:    return Token::BECOMES;
        case INT:        return Token::INT;
        case EQ:         return Token::EQ;
        case NE:         return Token::NE;
        case LT:         return Token::LT;
        case GT:         return Token::GT;
        case LE:         return Token::LE;
        case GE:         return Token::GE;
        case PLUS:       return Token::PLUS;
        case MINUS:      return Token::MINUS;
        case STAR:       return Token::STAR;
        case SLASH:      return Token::SLASH;
        case PCT:        return Token::PCT;
        case COMMA:      return Token::COMMA;
        case SEMI:       return Token::SEMI;
        case NEW:        return Token::NEW;
        case DELETE:     return Token::DELETE;
        case LBRACK:     return Token::LBRACK;
        case RBRACK:     return Token::RBRACK;
        case AMP:        return Token::AMP;
        case NULL_TYPE:  return Token::NULL_TYPE;
        case WHITESPACE: return Token::WHITESPACE;
        case COMMENT:    return Token::COMMENT;
        case ZERO:       return Token::NUM;
        default: throw ScanningFailure("ERROR: Cannot convert state to kind.");
      }
    }


  public:
    /* Tokenizes an input string according to the SMM algorithm.
     * You will learn the SMM algorithm in class around the time of Assignment 6.
     */
    std::vector<Token> simplifiedMaximalMunch(const std::string &input) const {
      std::vector<Token> result;

      State state = start();
      std::string munchedInput;

      // We can't use a range-based for loop effectively here
      // since the iterator doesn't always increment.
      for (std::string::const_iterator inputPosn = input.begin();
           inputPosn != input.end();) {

        State oldState = state;
        state = transition(state, *inputPosn);

        if (!failed(state)) {
          munchedInput += *inputPosn;
          oldState = state;

          ++inputPosn;
        }

        if (inputPosn == input.end() || failed(state)) {
          if (accept(oldState)) {
            result.push_back(Token(stateToKind(oldState), munchedInput));

            munchedInput = "";
            state = start();
          } else {
            if (failed(state)) {
              munchedInput += *inputPosn;
            }
            throw ScanningFailure("ERROR: Simplified maximal munch failed on input: "
                                 + munchedInput);
          }
        }
      }

      return result;
    }

    /* Initializes the accepting states for the DFA.
     */
    wlp4DFA() {
      acceptingStates = {ZERO, LPAREN, RPAREN, LBRACE, RBRACE, PLUS,
                         MINUS, STAR, PCT, COMMA, SEMI, LBRACK, RBRACK,
                         AMP, ID, NUM, BECOMES, EQ, NE, LT, LE, GT, GE,
                         SLASH, COMMENT, WHITESPACE};

      // Initialize transitions for the DFA
      for (size_t i = 0; i < transitionFunction.size(); ++i) {
        for (size_t j = 0; j < transitionFunction[0].size(); ++j) {
          transitionFunction[i][j] = FAIL;
        }
      }

      // Low-hanging fruit
      registerTransition(START, "0", ZERO);
      registerTransition(START, "(", LPAREN);
      registerTransition(START, ")", RPAREN);
      registerTransition(START, "{", LBRACE);
      registerTransition(START, "}", RBRACE);
      registerTransition(START, "+", PLUS);
      registerTransition(START, "-", MINUS);
      registerTransition(START, "*", STAR);
      registerTransition(START, "%", PCT);
      registerTransition(START, ",", COMMA);
      registerTransition(START, ";", SEMI);
      registerTransition(START, "]", LBRACK);
      registerTransition(START, "[", RBRACK);
      registerTransition(START, "&", AMP);
      // ID (check for keywords later)
      registerTransition(START, isalpha, ID);
      registerTransition(ID, isalnum, ID);
      // Integer (check for its value later)
      registerTransition(START, "123456789", NUM);
      registerTransition(NUM, isdigit, NUM);
      // Equal
      registerTransition(START, "=", BECOMES);
      registerTransition(BECOMES, "=", EQ);
      // Comparison
      registerTransition(START, "!", GanTanHao);
      registerTransition(GanTanHao, "=", NE);
      registerTransition(START, "<", LT);
      registerTransition(LT, "=", LE);
      registerTransition(START, ">", GT);
      registerTransition(GT, "=", GE);
      // Single slash and double slash
      registerTransition(START, "/", SLASH);
      registerTransition(SLASH, "/", COMMENT);
      registerTransition(COMMENT, [](int c) -> int { return c != '\n'; }, COMMENT);
      // Whitespace 
      registerTransition(START, isspace, WHITESPACE);
      registerTransition(WHITESPACE, isspace, WHITESPACE);
    }

    // Register a transition on all chars in chars
    void registerTransition(State oldState, const std::string &chars,
        State newState) {
      for (char c : chars) {
        transitionFunction[oldState][c] = newState;
      }
    }

    // Register a transition on all chars matching test
    // For some reason the cctype functions all use ints, hence the function
    // argument type.
    void registerTransition(State oldState, int (*test)(int), State newState) {

      for (int c = 0; c < 128; ++c) {
        if (test(c)) {
          transitionFunction[oldState][c] = newState;
        }
      }
    }

    /* Returns the state corresponding to following a transition
     * from the given starting state on the given character,
     * or a special fail state if the transition does not exist.
     */
    State transition(State state, char nextChar) const {
      return transitionFunction[state][nextChar];
    }

    /* Checks whether the state returned by transition
     * corresponds to failure to transition.
     */
    bool failed(State state) const { return state == FAIL; }

    /* Checks whether the state returned by transition
     * is an accepting state.
     */
    bool accept(State state) const {
      return acceptingStates.count(state) > 0;
    }

    /* Returns the starting state of the DFA
     */
    State start() const { return START; }
};

std::vector<Token> scan(const std::string &input) {
  static wlp4DFA theDFA;

  std::vector<Token> tokens = theDFA.simplifiedMaximalMunch(input);

  // We need to:
  // * Throw exceptions for WORD tokens whose lexemes aren't ".word".
  // * Remove WHITESPACE and COMMENT tokens entirely.

  std::vector<Token> newTokens;

  for (auto token = newTokens.begin(); token != newTokens.end(); ++token) {
    if (token->getKind() == Token::ID) {
      if (token->getLexeme() == "return") {
        newTokens.push_back(Token(Token::RETURN, "return"));
      } else if (token->getLexeme() == "if") {
        newTokens.push_back(Token(Token::IF, "if"));
      } else if (token->getLexeme() == "else") {
        newTokens.push_back(Token(Token::ELSE, "else"));
      } else if (token->getLexeme() == "while") {
        newTokens.push_back(Token(Token::WHILE, "while"));
      } else if (token->getLexeme() == "println") {
        newTokens.push_back(Token(Token::PRINTLN, "println"));
      } else if (token->getLexeme() == "wain") {
        newTokens.push_back(Token(Token::WAIN, "wain"));
      } else if (token->getLexeme() == "int") {
        newTokens.push_back(Token(Token::INT, "int"));
      } else if (token->getLexeme() == "new") {
        newTokens.push_back(Token(Token::NEW, "new"));
      } else if (token->getLexeme() == "delete") {
        newTokens.push_back(Token(Token::DELETE, "delete"));
      } else if (token->getLexeme() == "NULL") {
        newTokens.push_back(Token(Token::NULL_TYPE, "NULL"));
      } else {
        newTokens.push_back(Token(Token::ID, token->getLexeme()));
      }
    } else if (token->getKind() == Token::NUM) {
        std::string value = token->getLexeme();
            

        if (value.length() > 10) {
            throw ScanningFailure("ERROR: Numeric literal out of range");
        }
        int64_t val = token->toLong();
        int64_t bound = 2147483647;
        if (val > bound) {
            throw ScanningFailure("ERROR: Numeric literal out of range");
        }
        newTokens.push_back(*token);
    } else if (token->getKind() != Token::WHITESPACE && token->getKind() != Token::Kind::COMMENT) {
        newTokens.push_back(*token);
    }
  }

  return newTokens;
}
