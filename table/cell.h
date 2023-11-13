#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell();
    
    void Set(std::string text);
    void Clear();
    
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    
    void ClearCache();
    
private:
    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
       
        virtual ~Impl() = default;
    };
    class EmptyImpl: public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };
    class TextImpl: public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;
        
    private:
        std::string text_;
    };
    class FormulaImpl: public Impl {
    public:
        explicit FormulaImpl(std::string formula, SheetInterface& sheet);
        
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        
    private:
        std::unique_ptr<FormulaInterface> formula_;
        SheetInterface& sheet_;
    };

    std::unique_ptr<Impl> impl_;
    
    std::unordered_set<Cell*> cells_used_for_calculation_; // Ячейки используемые текущей ячейкой.
    std::unordered_set<Cell*> cells_using_this_cell_; // Ячейки использующие текущую ячейку.

    mutable std::optional<Value> cache_;

    Sheet& sheet_;
    
    
    bool IsCircularDependency(const Impl* impl) const;
};
