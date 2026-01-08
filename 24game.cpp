#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <set>
#include <numeric>
#include <limits>
#include <algorithm>

#define REDIRECT_IO std::freopen("input.txt", "r", stdin); std::freopen("output.txt", "w", stdout); std::freopen("error.txt", "w", stderr)

enum OperationName {SUM, DIFF, PROD, DIV1, DIV2, POW1, POW2, EXP, LOG,};
const int dividedByZero{ -1'0000'0000 };
//const int magicNumber { 24 };  // changed to a local interger in main(), optionally adjustable from user input 

struct Fraction
{  // always reduced to its lowest term with positive denominator
    int numerator{1};    // set to be 1 if actually an integer
    int denominator{1};
    
    // our customised constructor normalizes sign and reduces, but its inherit class then needs some care
    //Fraction(int n=1, int d=1) : numerator(n), denominator(d) {
    //    if (denominator == 0) throw std::invalid_argument("denominator is zero");
    //    if (denominator < 0) { numerator = -numerator; denominator = -denominator; }
    //    int t_gcd = std::gcd(std::abs(numerator), denominator);
    //    numerator /= t_gcd;
    //    denominator /= t_gcd;
    //    if (numerator == 0) denominator = 1;  // Handle zero: 0/x → 0/1
    //}

    // C++20: Three-way comparison (provides ==, !=, <, <=, >, >= automatically)
    //auto operator<=>(const Fraction& other) const {
    //    auto lhs = numerator * other.denominator;
    //    auto rhs = other.numerator * denominator;
    //    return lhs <=> rhs;
    //}
    
    // pre-C++20 style, define equality, less-than, ..., individually 
    //bool operator==(const Fraction& other) const {
    //    return numerator == other.numerator && denominator == other.denominator;
    //}

    // define less-than with a cheap comparasion, not overflow safe
    bool operator<(const Fraction& other) const {
        return numerator * other.denominator < other.numerator * denominator;
    }   // for now we only need less-than in the code

    // the rest four relational operators can all be derived from above two
    //bool operator!=(const Fraction& other) const { return !(*this == other); }
    //bool operator<=(const Fraction& other) const { return *this < other || *this == other; }
    //bool operator> (const Fraction& other) const { return other < *this; }
    //bool operator>=(const Fraction& other) const { return !(*this < other); }
};

struct Operand : Fraction
{
    std::string textPrint{ '1' };
};

Operand sumn(const Operand& op1, const Operand& op2)
{
    Operand r;
    if (op1.denominator == 1 && op2.denominator == 1) {
        //r.denominator = 1;  // already initialised as 1
        r.numerator = op1.numerator + op2.numerator;
    } else {
        int denom = std::lcm(op1.denominator, op2.denominator);
        int numer = op1.numerator * (denom/op1.denominator) + op2.numerator * (denom/op2.denominator);
        int gcd_n = std::gcd(denom, numer);
        r.numerator = numer / gcd_n;
        r.denominator = denom / gcd_n;
    }
    
    char operChar{ '+' };
    r.textPrint = '(' + op1.textPrint + operChar + op2.textPrint + ')'; 

    return r;
}

Operand diff(const Operand& op1, const Operand& op2)
{  // we could swap op1 and op2 now, as the line below, but this is less optimal,
   // because this not only duplicates the effort, but also risks integer overflow
    //if (op1.numerator * op2.denominator < op2.numerator * op1.denominator) { Operand t = op1; op1 = op2; op2 = t; }
    //Or we could do recursive call once as below. But we have already guaranteed the order
    //if (op1 < op2) return diff(op2, op1);  // if op1 < op2 indeed, this will compare them twice! 
    
    Operand r;
    if (op1.denominator == 1 && op2.denominator == 1) {
        //r.denominator = 1;
        r.numerator = op1.numerator - op2.numerator;
    } else {
        int denom = std::lcm(op1.denominator, op2.denominator);
        int numer = op1.numerator * (denom/op1.denominator) - op2.numerator * (denom/op2.denominator);
        int gcd_n = std::gcd(denom, numer);
        r.numerator = numer / gcd_n;
        r.denominator = denom / gcd_n;
    }

    char operChar{ '-' };    
    if (r.numerator < 0) {  // better approach than swapping the order of op1 and op2 as above
        r.numerator = -r.numerator;
        //Operand t = op1; op1 = op2; op2 = t;  // swap their order here just for correct textPrint
        r.textPrint = '(' + op2.textPrint + operChar + op1.textPrint + ')';
    } else {  // with texPrint now directly produced, we can potentially use constant parameters, i.e., not to swap the order of op1 and op2 at all
        r.textPrint = '(' + op1.textPrint + operChar + op2.textPrint + ')';
    }

    return r;
}

Operand prod(const Operand& op1, const Operand& op2)
{
    Operand r;
    if (op1.denominator == 1 && op2.denominator == 1) {
        //r.denominator = 1;
        r.numerator = op1.numerator * op2.numerator;
    } else {
        int gcd_1 = std::gcd(op1.numerator, op2.denominator);
        int gcd_2 = std::gcd(op2.numerator, op1.denominator);
        int denom = (op1.denominator/gcd_2) * (op2.denominator/gcd_1);
        int numer = (op1.numerator/gcd_1) * (op2.numerator/gcd_2);
        int gcd_n = std::gcd(denom, numer);
        r.numerator = numer / gcd_n;
        r.denominator = denom / gcd_n;
    }
    
    char operChar{ '*' };
    r.textPrint = '(' + op1.textPrint + operChar + op2.textPrint + ')';

    return r;
}

Operand divn(const Operand& op1, const Operand& op2)
{  // we could flip op2's numerator and denominator and call the prod() function above,
   // but it is then awkard to retrive the textPrint, especially if op2 is generated
   // Another reason is that the error of dividing zero can be handled at one place

    Operand r{ dividedByZero, 1, "(***DividedByZeroError***)" };
    if (op2.numerator == 0) return r;
    
    if (op1.denominator == 1 && op2.numerator == 1) {
        //r.denominator = 1;
        r.numerator = op1.numerator * op2.denominator;
    } else {
        int gcd_1 = std::gcd(op1.numerator, op2.numerator);
        int gcd_2 = std::gcd(op1.denominator, op2.denominator);
        int denom = (op1.denominator/gcd_2) * (op2.numerator/gcd_1);
        int numer = (op1.numerator/gcd_1) * (op2.denominator/gcd_2);
        int gcd_n = std::gcd(denom, numer);
        r.numerator = numer / gcd_n;
        r.denominator = denom / gcd_n;
    }
    
    char operChar{ r.denominator == 1 ? '/' : ':' };  // use a different char to note unclean divisions 
    r.textPrint = '(' + op1.textPrint + operChar + op2.textPrint + ')';

    return r;
}

Operand operation(OperationName operNm, const Operand& op1, const Operand& op2)
{
    const Operand notSupported{ -1, 1, "(***UnkownOperationError***)" };
    switch(operNm)
    {
    case SUM:
        return sumn(op1, op2);
    case DIFF:
        return diff(op1, op2);
    case PROD:
        return prod(op1, op2);
    case DIV1:
        return divn(op1, op2);
    case DIV2:
        return divn(op2, op1);
    default:  // to extend the support of other kinds of operations here 
        return notSupported;
    }
}

std::array<int, 4> getFourPositiveNumbers()
{
    std::array<int, 4> a;
    for (int i{0}; i<4; ++i) {
        if (!((std::cin >> a[i]) && (a[i] > 0))) {
#ifdef BATCHMODE
            std::cerr << "Non-positive-integer entry in batch input file!\n";
            throw std::invalid_argument("Invalid Data Input.");
            //std::exit(-1);
#else
            std::cout << "Your number is invalid. Please try to enter this number again, and continue your supply of four positive integers: ";
            std::cin.clear(); // clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip bad input in the rest of the line
            --i;  // decrement i so as to retry from the same index
            continue;
#endif
        }
    }
    std::sort(std::begin(a), std::end(a), std::greater<int>());
    return a;
}

int main(int argc, char *argv[])
{
    OperationName legitimateOperations[] = {SUM, DIFF, PROD, DIV1, DIV2};
    int magicNumber{ 24 };  // changed from a global constant to a local variable, optionally adjustable from user input
    bool printOut{ true };
    int test{1};
#ifdef BATCHMODE
    REDIRECT_IO;
    std::cin >> magicNumber;  // may be overwritten by a commandline input, see below
    std::cin >> printOut;
    std::cin >> test;
#else
    std::cout << "Enter four positive integers: ";
#endif
    if (argc >= 2) {
        magicNumber = std::stoi(argv[1]); // overwrite the target magicNumber
    }  // no exception handling, assuming valid user input

    while (test--) {
        std::array<int, 4> numbers;
        try {
            numbers = getFourPositiveNumbers();
        } catch (const std::invalid_argument& e) {
            std::cerr << "Aborted due to " << e.what() << '\n';
            std::exit(-1);
        }
        
        Operand x[4];
        for (int i{0}; i<4; ++i) { x[i].numerator = numbers[i]; x[i].textPrint = std::to_string(numbers[i]); }
       
        bool foundSome{ false };
        std::set<std::string> allSolutions{};

        for (int i{0}; i<3; ++i) {
            for (int j{i+1}; j<4; ++j) {
                int m{ -1 }; int n{ -1 };
                for (int trigger{0}, k{0}; k<4; ++k) { 
                    if (k != i && k != j) (trigger++ == 0 ? m : n) = k;
                }    // the other two numbers now have indices m and n
                for (OperationName firstOpNm : legitimateOperations) {
                    Operand firstOpRst = operation(firstOpNm, x[i], x[j]);
                    if (firstOpRst.numerator < 0) continue;  // error of being divided by zero, or other unsupported operation, therefore move to next iteration
                    
                    for (int k{0}; k<3; ++k) {
                        Operand *yOp1{}, *yOp2{}, *zOpKeeper{};  // they are all nullptr, but will be assigned next
                        switch(k)
                        {
                        case 0:
                            zOpKeeper = &firstOpRst;
                            if (x[m] < x[n]) { yOp1 = &x[n]; yOp2 = &x[m]; }
                            else { yOp1 = &x[m]; yOp2 = &x[n]; }  // xm, xn now in order
                            break;
                        case 1:
                            zOpKeeper = &x[n];
                            if (x[m] < firstOpRst) { yOp1 = &firstOpRst; yOp2 = &x[m]; }
                            else { yOp1 = &x[m]; yOp2 = &firstOpRst; }  // xm, 1stoprst in order
                            break;
                        case 2:
                            zOpKeeper = &x[m];
                            if (x[n] < firstOpRst) { yOp1 = &firstOpRst; yOp2 = &x[n]; }
                            else { yOp1 = &x[n]; yOp2 = &firstOpRst; }  // xn, 1stoprst in order
                            break;
                        }  // now all three pointers are 
                        
                        for (OperationName secondOpNm : legitimateOperations) {
                            Operand secondOpRst = operation(secondOpNm, *yOp1, *yOp2);
                            if (secondOpRst.numerator < 0) continue;  // got error, skip and move to next iteration
                            
                            Operand *zOp1{}, *zOp2{};  // we assign these pointers next
                            if (*zOpKeeper < secondOpRst) { zOp1 = &secondOpRst; zOp2 = zOpKeeper; }
                            else { zOp1 = zOpKeeper; zOp2 = &secondOpRst; }   // *zOpKeepr, secondOpRst now in order
                            for (OperationName lastOpNm : legitimateOperations) {
                                Operand lastOpRst = operation(lastOpNm, *zOp1, *zOp2);
                                if (lastOpRst.numerator < 0) continue;  // the same error skip
                                if (lastOpRst.denominator == 1 && lastOpRst.numerator == magicNumber) {  // may extend operator== for fractions and use it here, but why bother for this simple one-off action?
                                    if (!foundSome) foundSome = true;
                                    std::string successfulCalculation = lastOpRst.textPrint + '=' + std::to_string(magicNumber) + '\n';
                                    allSolutions.insert(successfulCalculation);
                                }  //else lastOpRst.denominator == 1 ? cerr << lastOpRst.numerator << '\n' : cerr << lastOpRst.numerator << ':' << lastOpRst.denominator << '\n';
                            }
                        }
                    }
                }
            }
        }
        std::cout << std::setw(2) << numbers[0] << std::setw(3) << numbers[1] << std::setw(3) << numbers[2] << std::setw(3) << numbers[3] << " | ";
        if (foundSome && printOut) { std::cout << "Y> Solutions are:\n"; for (std::string s : allSolutions) std::cout << s; }
        else if (foundSome) std::cout << "Y> Is solvable.\n";
        else std::cout << "N> Not found.\n";
    }

    std::cout << "Done.\n";
    return 0;
}
