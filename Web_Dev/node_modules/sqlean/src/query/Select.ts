import { Statement } from "better-sqlite3";

import { DB } from "../Db";
import { Util } from "../Util";
import { Where, WhereBuilder } from "./Where";

export type SelectColumns = string[] | '*';

export type OrderType = 'ASC' | 'DESC';
export type Order = { [column: string]: OrderType }

export class SelectData
{
    table:      string;
    columns:    SelectColumns;

    where?:     Where;
    
    order?:     Order = {};

    limit?:     number;
    offset?:    number;

    pluck? = false;
}

export class PreparedSelect
{
    statement:      Statement;
    whereParams:    Object;
}

export class Select
{
    static Prepare(data: SelectData): PreparedSelect
    {
        let result = new PreparedSelect;
        data = {...new SelectData, ...data};

        let sqlColumns =    Select.getSqlColumns(data.columns);
        let whereResults =  new WhereBuilder(data.where || '').getResult();
        let sqlOrder =      Select.getSqlOrder(data.order);

        let sql = `
            SELECT  ${sqlColumns}
            FROM    ${data.table}
            ${whereResults.sql}
            ${sqlOrder}
            ${typeof data.limit === 'number'    ? `LIMIT ${data.limit}` : ''}
            ${typeof data.offset === 'number'   ? `OFFSET ${data.offset}` : ''}
        `;

        // For Debug
        // console.log(sql);

        result.statement =      DB._db.prepare(sql).pluck(data.pluck);
        result.whereParams =    whereResults.params;

        return result;
    }

    static Get(data: SelectData): any
    {
        let prepareResult = this.Prepare(data);
        let result = prepareResult.statement.get(prepareResult.whereParams) || null;

        return Util.objToCamelCase(result);
    }

    static All(data: SelectData): any[]
    {
        let prepareResult = this.Prepare(data);
        let result = prepareResult.statement.all(prepareResult.whereParams);

        if (result.length === 0)
            return null;

        return result.map(row => Util.objToCamelCase(row));
    }

    //
    // 
    //

    static getSqlColumns(columns: SelectColumns): string
    {
        if (!Array.isArray(columns)) columns = [columns];
        return columns.map(column => Util.toSnakeCase(column)).join(', ');
    }

    static getSqlOrder(order: Order)
    {
        let sql = '';

        Object.keys(order).forEach((column, i, arr) =>
        {
            sql += ` ${Util.toSnakeCase(column)} ${order[column]}`;

            if (i !== arr.length - 1)
                sql += ',';
        });

        if (sql !== '')
            sql = 'ORDER BY ' + sql;

        return sql;
    }
}