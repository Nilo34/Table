#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression);
    Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    FormulaAST ast_;
};

Formula::Formula(std::string expression)
try
: ast_(ParseFormulaAST(expression))
{
    //ast_.PrintCells(std::cout);
}
catch (...) {
    throw FormulaException("Invalid formula");
}

Formula::Value Formula::Evaluate(const SheetInterface& sheet) const {
    try {
        
        std::function<double(Position pos)> args = [&sheet] (const Position pos)->double {
            
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            } else {
                
                const auto* cell = sheet.GetCell(pos);
                
                if (!cell) {
                    return 0.0;
                } else {
                    
                    auto result = cell->GetValue();
                    
                    if (std::holds_alternative<double>(result)) {
                        return std::get<double>(result);
                    } else if (std::holds_alternative<std::string>(result)) {
                        std::string result_str = std::get<std::string>(result);
                        
                        if (result_str.empty()) {
                            return 0.0;
                        }
                        
                        size_t err_pos;
                        double result_doub = std::stod(result_str, &err_pos);

                        if (err_pos < result_str.length()) {
                            // Обработка ошибки преобразования строки в число
                            throw FormulaError(FormulaError::Category::Value);
                        }
                        
                        return result_doub;
                        
                    } else {
                        throw FormulaError(std::get<FormulaError>(result));
                    }
                    
                }
                
            }
            
        };
        
        return ast_.Execute(args);
        
    } catch (const FormulaError& formula_error) {
        return formula_error;
    }
}

std::string Formula::GetExpression() const {
    //Кажется это костыль, но другие советы из интернета выглядят слишком страшно
    std::ostringstream out;
    ast_.PrintFormula(out);
    return out.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    std::vector<Position> result;
    
    for (const Position& pos : ast_.GetCells()) {
        if (pos.IsValid()) {
            result.push_back(pos);
        }
    }
    
    return result;
}

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
