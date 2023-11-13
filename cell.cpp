#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
: impl_(std::make_unique<EmptyImpl>())
, sheet_(sheet)
{
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    size_t text_size = text.size();
    
    if (text_size == 0) {
        
        impl_ = std::make_unique<EmptyImpl>();
        
    } else if ((text.at(0) == FORMULA_SIGN) && (text_size >= 2)) {
        
        std::unique_ptr<FormulaImpl> buffer = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        
        if (IsCircularDependency(buffer.get())) {
            throw CircularDependencyException("The formula contains a cyclic dependence");
        }
        
        impl_ = std::move(buffer);
        
    } else {
        
        impl_ = std::move(std::make_unique<TextImpl>(std::move(text)));
        
    }
    
    
    for (Cell* cell_used_for_calculation : cells_used_for_calculation_) {
        cell_used_for_calculation->cells_using_this_cell_.erase(this);
    }

    cells_used_for_calculation_.clear();

    for (const Position& position : impl_->GetReferencedCells()) {

        Cell* cell_used_for_calculation = sheet_.GetConcreteCell(position);

        if (!cell_used_for_calculation) {
            sheet_.SetCell(position, "");
            cell_used_for_calculation = sheet_.GetConcreteCell(position);
        }

        cells_used_for_calculation_.insert(cell_used_for_calculation);
        cell_used_for_calculation->cells_using_this_cell_.insert(this);
    }
    
    
    ClearCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}
std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}
    
void Cell::ClearCache() {
    if (cache_) {
        
        cache_.reset();
        
        for (Cell* cell : cells_using_this_cell_) {
            cell->ClearCache();
        }
        
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}
std::string Cell::EmptyImpl::GetText() const {
    return "";
}

Cell::TextImpl::TextImpl(std::string text) 
: text_(std::move(text))
{
}
Cell::Value Cell::TextImpl::GetValue() const {
    
    if ((text_.size() != 0) && (text_.at(0) == ESCAPE_SIGN)) {
        return text_.substr(1);
    }
    
    return text_;
}
std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) 
: formula_(ParseFormula(text.substr(1)))
, sheet_(sheet)
{
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto result = formula_->Evaluate(sheet_);
    
    if (std::holds_alternative<double>(result)) {
        return std::get<double>(result);
    }
    
    return std::get<FormulaError>(result);
}
std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool Cell::IsCircularDependency(const Cell::Impl* impl) const {
    
    const std::vector<Position> vector_of_cells_mentioned_in_the_formula = impl->GetReferencedCells();
        
        if (!vector_of_cells_mentioned_in_the_formula.empty()) {
            
            std::unordered_set<const Cell*> set_of_cells_used;
            
            for (const Position& position : vector_of_cells_mentioned_in_the_formula) {
                set_of_cells_used.insert(sheet_.GetConcreteCell(position));
            }
            
            std::unordered_set<const Cell*> set_of_visited_cells;
            std::vector<const Cell*> vector_of_cells_to_visit;
            
            vector_of_cells_to_visit.push_back(this);
            
            while (!vector_of_cells_to_visit.empty()) {
                
                const Cell* cell_to_be_checked = vector_of_cells_to_visit.back();
                vector_of_cells_to_visit.pop_back();
                
                set_of_visited_cells.insert(cell_to_be_checked);
                
                if (set_of_cells_used.find(cell_to_be_checked) != set_of_cells_used.end()) {
                    return true;
                }
                
                for (const Cell* cell_used_for_calculation : cell_to_be_checked->cells_used_for_calculation_) {
                    if (set_of_visited_cells.find(cell_used_for_calculation) == set_of_visited_cells.end()) {
                        vector_of_cells_to_visit.push_back(cell_used_for_calculation);
                    }
                }
                
            }
            
        }
    
    return false;
}