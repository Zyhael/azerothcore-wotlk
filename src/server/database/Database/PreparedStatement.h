/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#ifndef _PREPAREDSTATEMENT_H
#define _PREPAREDSTATEMENT_H

#include "SQLOperation.h"
#include <ace/Future.h>

#ifdef __APPLE__
#undef TYPE_BOOL
#endif

//- Union for data buffer (upper-level bind -> queue -> lower-level bind)
union PreparedStatementDataUnion
{
    bool boolean;
    uint8 ui8;
    int8 i8;
    uint16 ui16;
    int16 i16;
    uint32 ui32;
    int32 i32;
    uint64 ui64;
    int64 i64;
    float f;
    double d;
};

//- This enum helps us differ data held in above union
enum PreparedStatementValueType
{
    TYPE_BOOL,
    TYPE_UI8,
    TYPE_UI16,
    TYPE_UI32,
    TYPE_UI64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_BINARY,
    TYPE_NULL
};

struct PreparedStatementData
{
    PreparedStatementDataUnion data;
    PreparedStatementValueType type;
    std::vector<uint8> binary;
};

//- Forward declare
class MySQLPreparedStatement;

//- Upper-level class that is used in code
class PreparedStatement
{
    friend class PreparedStatementTask;
    friend class MySQLPreparedStatement;
    friend class MySQLConnection;

public:
    explicit PreparedStatement(uint32 index);
    ~PreparedStatement();

    void setBool(const uint8 index, const bool value);
    void setUInt8(const uint8 index, const uint8 value);
    void setUInt16(const uint8 index, const uint16 value);
    void setUInt32(const uint8 index, const uint32 value);
    void setUInt64(const uint8 index, const uint64 value);
    void setInt8(const uint8 index, const int8 value);
    void setInt16(const uint8 index, const int16 value);
    void setInt32(const uint8 index, const int32 value);
    void setInt64(const uint8 index, const int64 value);
    void setFloat(const uint8 index, const float value);
    void setDouble(const uint8 index, const double value);
    void setString(const uint8 index, const std::string& value);
    void setBinary(const uint8 index, const std::vector<uint8>& value);
    template<size_t Size>
    void setBinary(const uint8 index, std::array<uint8, Size> const& value)
    {
        std::vector<uint8> vec(value.begin(), value.end());
        setBinary(index, vec);
    }
    void setNull(const uint8 index);

protected:
    void BindParameters();

protected:
    MySQLPreparedStatement* m_stmt;
    uint32 m_index;
    std::vector<PreparedStatementData> statement_data;    //- Buffer of parameters, not tied to MySQL in any way yet
};

//- Class of which the instances are unique per MySQLConnection
//- access to these class objects is only done when a prepared statement task
//- is executed.
class MySQLPreparedStatement
{
    friend class MySQLConnection;
    friend class PreparedStatement;

public:
    MySQLPreparedStatement(MYSQL_STMT* stmt);
    ~MySQLPreparedStatement();

    void setBool(const uint8 index, const bool value);
    void setUInt8(const uint8 index, const uint8 value);
    void setUInt16(const uint8 index, const uint16 value);
    void setUInt32(const uint8 index, const uint32 value);
    void setUInt64(const uint8 index, const uint64 value);
    void setInt8(const uint8 index, const int8 value);
    void setInt16(const uint8 index, const int16 value);
    void setInt32(const uint8 index, const int32 value);
    void setInt64(const uint8 index, const int64 value);
    void setFloat(const uint8 index, const float value);
    void setDouble(const uint8 index, const double value);
    void setBinary(const uint8 index, const std::vector<uint8>& value, bool isString);
    void setNull(const uint8 index);

protected:
    MYSQL_STMT* GetSTMT() { return m_Mstmt; }
    MYSQL_BIND* GetBind() { return m_bind; }
    PreparedStatement* m_stmt;
    void ClearParameters();
    bool CheckValidIndex(uint8 index);
    [[nodiscard]] std::string getQueryString(std::string const& sqlPattern) const;

private:
    void setValue(MYSQL_BIND* param, enum_field_types type, const void* value, uint32 len, bool isUnsigned);

private:
    MYSQL_STMT* m_Mstmt;
    uint32 m_paramCount;
    std::vector<bool> m_paramsSet;
    MYSQL_BIND* m_bind;
};

typedef ACE_Future<PreparedQueryResult> PreparedQueryResultFuture;

//- Lower-level class, enqueuable operation
class PreparedStatementTask : public SQLOperation
{
public:
    PreparedStatementTask(PreparedStatement* stmt);
    PreparedStatementTask(PreparedStatement* stmt, PreparedQueryResultFuture result);
    ~PreparedStatementTask() override;

    bool Execute() override;

protected:
    PreparedStatement* m_stmt;
    bool m_has_result;
    PreparedQueryResultFuture m_result;
};

#endif
