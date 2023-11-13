#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (pos.IsValid()) {
        
        cells_.resize(std::max(pos.row + 1, int(std::size(cells_))));
        cells_[pos.row].resize(std::max(pos.col + 1, int(std::size(cells_[pos.row]))));
        
        if (!cells_[pos.row][pos.col]) {
            cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        }
        
        cells_[pos.row][pos.col]->Set(text);
        
    } else {
        throw InvalidPositionException("invalid position");
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid()) {
        
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            if (cells_[pos.row][pos.col]->GetText() != "") {
                return cells_[pos.row][pos.col].get();
            }
        }
        
        return nullptr;
        
    } else {
        throw InvalidPositionException("invalid position");
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    if (pos.IsValid()) {
        
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            if (cells_[pos.row][pos.col]->GetText() != "") {
                return cells_[pos.row][pos.col].get();
            }
        }
        
        return nullptr;
        
    } else {
        throw InvalidPositionException("invalid position");
    }
}

Cell* Sheet::GetConcreteCell(Position pos) {
    if (pos.IsValid()) {
        
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            return cells_[pos.row][pos.col].get();
        }
        
        return nullptr;
        
    } else {
        throw InvalidPositionException("invalid position");
    }
}

void Sheet::ClearCell(Position pos) {
    if (pos.IsValid()) {
        
        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col]) {
                cells_[pos.row][pos.col]->Clear();
            }
            
        }
        
    } else {
        throw InvalidPositionException("invalid position");
    }
}

Size Sheet::GetPrintableSize() const {
    Size result;
    
    for (int y = 0; y < int(std::size(cells_)); ++y) {
        for (int x = 0; x < int(std::size(cells_[y])); ++x) {
            
            if (cells_[y][x]) {
                if (cells_[y][x]->GetText() != "") {
                    result.rows = std::max(result.rows, y + 1);
                    result.cols = std::max(result.cols, x + 1);
                }
            }
            
        }
    }
    
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    
    for (int y = 0; y < size.rows; ++y) {
        for (int x = 0; x < size.cols; ++x) {
            
            if (x > 0) {
                output << '\t';
            }
            
            if (x < int(std::size(cells_[y]))) {
                if (cells_[y][x]) {
                    std::visit([&output] (const auto& val) {output << val;},
                               cells_[y][x]->GetValue());
                }
            }
            
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    
    for (int y = 0; y < size.rows; ++y) {
        for (int x = 0; x < size.cols; ++x) {
            
            if (x > 0) {
                output << '\t';
            }
            
            if (x < int(std::size(cells_[y]))) {
                if (cells_[y][x]) {
                    output << cells_[y][x]->GetText();
                }
            }
            
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
