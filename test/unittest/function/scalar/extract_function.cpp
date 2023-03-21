//
// Created by jinhai on 23-3-19.
//


#include <gtest/gtest.h>
#include "base_test.h"
#include "common/column_vector/column_vector.h"
#include "common/types/value.h"
#include "main/logger.h"
#include "main/stats/global_resource_usage.h"
#include "main/infinity.h"
#include "common/types/info/varchar_info.h"
#include "storage/catalog.h"
#include "function/scalar/extract.h"
#include "function/scalar_function_set.h"
#include "expression/column_expression.h"

class ExtractFunctionTest : public BaseTest {
    void
    SetUp() override {
        infinity::GlobalResourceUsage::Init();
        infinity::Infinity::instance().Init();
    }

    void
    TearDown() override {
        infinity::Infinity::instance().UnInit();
        EXPECT_EQ(infinity::GlobalResourceUsage::GetObjectCount(), 0);
        EXPECT_EQ(infinity::GlobalResourceUsage::GetRawMemoryCount(), 0);
        infinity::GlobalResourceUsage::UnInit();
    }
};

TEST_F(ExtractFunctionTest, extract_year_test) {
    using namespace infinity;

    UniquePtr<Catalog> catalog_ptr = MakeUnique<Catalog>();

    RegisterExtractFunction(catalog_ptr);

    {
        String op = "extract_year";
        SharedPtr<FunctionSet> function_set = catalog_ptr->GetFunctionSetByName(op);
        EXPECT_EQ(function_set->type_, FunctionType::kScalar);
        SharedPtr<ScalarFunctionSet> scalar_function_set = std::static_pointer_cast<ScalarFunctionSet>(function_set);

        Vector<SharedPtr<BaseExpression>> inputs;

        DataType data_type(LogicalType::kDate);
        SharedPtr<ColumnExpression> col_expr_ptr = MakeShared<ColumnExpression>(data_type,
                                                                                "t1",
                                                                                1,
                                                                                "c1",
                                                                                0,
                                                                                0);

        inputs.emplace_back(col_expr_ptr);
        ScalarFunction func = scalar_function_set->GetMostMatchFunction(inputs);
        EXPECT_STREQ("extract_year(Date)->BigInt", func.ToString().c_str());

        Vector<DataType> column_types;
        column_types.emplace_back(data_type);

        SizeT row_count = DEFAULT_VECTOR_SIZE;

        DataBlock data_block;
        data_block.Init(column_types);

        for (SizeT i = 0; i < row_count; ++i) {
            DateT date_value;
            std::stringstream ss;
            ss << i + 1 << "-1-1";
            date_value.FromString(ss.str());
            data_block.AppendValue(0, Value::MakeDate(date_value));
        }
        data_block.Finalize();

        for (SizeT i = 0; i < row_count; ++i) {
            Value v = data_block.GetValue(0, i);
            EXPECT_EQ(v.type_.type(), LogicalType::kDate);
            std::stringstream ss;
            ss << i + 1 << "-01-01";
            EXPECT_EQ(v.value_.date.ToString(), ss.str());
        }

        DataType result_type(LogicalType::kBigInt);
        SharedPtr<ColumnVector> result = MakeShared<ColumnVector>(result_type);
        result->Initialize();
        func.function_(data_block, result);

        for (SizeT i = 0; i < row_count; ++i) {
            Value v = result->GetValue(i);
            EXPECT_EQ(v.type_.type(), LogicalType::kBigInt);
            EXPECT_EQ(v.value_.big_int, i + 1);
        }
    }

    {
        String op = "extract_month";
        SharedPtr<FunctionSet> function_set = catalog_ptr->GetFunctionSetByName(op);
        EXPECT_EQ(function_set->type_, FunctionType::kScalar);
        SharedPtr<ScalarFunctionSet> scalar_function_set = std::static_pointer_cast<ScalarFunctionSet>(function_set);

        Vector<SharedPtr<BaseExpression>> inputs;

        DataType data_type(LogicalType::kDate);
        SharedPtr<ColumnExpression> col_expr_ptr = MakeShared<ColumnExpression>(data_type,
                                                                                "t1",
                                                                                1,
                                                                                "c1",
                                                                                0,
                                                                                0);

        inputs.emplace_back(col_expr_ptr);
        ScalarFunction func = scalar_function_set->GetMostMatchFunction(inputs);
        EXPECT_STREQ("extract_month(Date)->BigInt", func.ToString().c_str());

        Vector<DataType> column_types;
        column_types.emplace_back(data_type);

        SizeT row_count = DEFAULT_VECTOR_SIZE;

        DataBlock data_block;
        data_block.Init(column_types);

        for (SizeT i = 0; i < row_count; ++i) {
            DateT date_value;
            std::stringstream ss;
            ss << i + 1 << "-" << i % 12 + 1 << "-1";
            date_value.FromString(ss.str());
            data_block.AppendValue(0, Value::MakeDate(date_value));
        }
        data_block.Finalize();

        DataType result_type(LogicalType::kBigInt);
        SharedPtr<ColumnVector> result = MakeShared<ColumnVector>(result_type);
        result->Initialize();
        func.function_(data_block, result);

        for (SizeT i = 0; i < row_count; ++i) {
            Value v = result->GetValue(i);
            EXPECT_EQ(v.type_.type(), LogicalType::kBigInt);
            EXPECT_EQ(v.value_.big_int, i % 12 + 1);
        }
    }

    {
        String op = "extract_day";
        SharedPtr<FunctionSet> function_set = catalog_ptr->GetFunctionSetByName(op);
        EXPECT_EQ(function_set->type_, FunctionType::kScalar);
        SharedPtr<ScalarFunctionSet> scalar_function_set = std::static_pointer_cast<ScalarFunctionSet>(function_set);

        Vector<SharedPtr<BaseExpression>> inputs;

        DataType data_type(LogicalType::kDate);
        SharedPtr<ColumnExpression> col_expr_ptr = MakeShared<ColumnExpression>(data_type,
                                                                                "t1",
                                                                                1,
                                                                                "c1",
                                                                                0,
                                                                                0);

        inputs.emplace_back(col_expr_ptr);
        ScalarFunction func = scalar_function_set->GetMostMatchFunction(inputs);
        EXPECT_STREQ("extract_day(Date)->BigInt", func.ToString().c_str());

        Vector<DataType> column_types;
        column_types.emplace_back(data_type);

        SizeT row_count = DEFAULT_VECTOR_SIZE;

        DataBlock data_block;
        data_block.Init(column_types);

        for (SizeT i = 0; i < row_count; ++i) {
            DateT date_value;
            std::stringstream ss;
            ss << "1-1" << "-" << i % 28 + 1;
            date_value.FromString(ss.str());
            data_block.AppendValue(0, Value::MakeDate(date_value));
        }
        data_block.Finalize();

        DataType result_type(LogicalType::kBigInt);
        SharedPtr<ColumnVector> result = MakeShared<ColumnVector>(result_type);
        result->Initialize();
        func.function_(data_block, result);

        for (SizeT i = 0; i < row_count; ++i) {
            Value v = result->GetValue(i);
            EXPECT_EQ(v.type_.type(), LogicalType::kBigInt);
            EXPECT_EQ(v.value_.big_int, i % 28 + 1);
        }
    }
}