import { DbValue } from "../Db";
import { Util } from "../Util";

export type WhereOperator = '<' | '<=' | '=' | '>=' | '>' | '!=' | 'LIKE' | 'IN';
export type WhereGroupOperator = 'AND' | 'OR';

export type WhereClause = [DbValue, WhereOperator, DbValue];
export type WhereStr = string;

export type WhereGroup = [WhereGroupOperator, Where[]];

export type Where = WhereStr | WhereClause | WhereGroup;

export class WhereResult
{
    sql: string;
    params: Object;
}

export class WhereBuilder
{
    _paramI = 0;

    sql = '';
    params = {};

    constructor(where: Where)
    {
        this.sql = this
                        .parseWhere(where)
                        .replace(/@([a-zA-Z]+)/gm, (match, column) => Util.toSnakeCase(column));;
    }

    getResult()
    {
        let result = new WhereResult;
            result.sql =    (this.sql === '' ? '' : 'WHERE ' + this.sql);
            result.params = this.params;
        
        return result;
    }

    //
    //
    //

    parseWhere(where: Where): string
    {
        if (typeof where === 'string')
            return where;
        
        if (where.length === 3)
            return this.parseClause(where as WhereClause);

        return this.parseGroup(where);
    }

    parseClause(clause: WhereClause): string
    {
        let left =  clause[0];
        let right = clause[2];

        let parseValue = (value: DbValue) =>
        {
            if (typeof value === 'string')
            {
                if (value[0] === '@')
                    return value;
                if (value[0] === '^')
                    return value.substring(1);
            }
            
            let out = '$param' + this._paramI;

            this.params['param' + this._paramI] = value;
            this._paramI++;

            return out;
        }

        left = parseValue(left);
        right = parseValue(right);

        let operator =  clause[1];

        return `${left} ${operator} ${right}`;
    }

    parseGroup(group: WhereGroup): string
    {
        let sql = '';

        group[1].forEach((where, i, arr) =>
        {
            sql += '(' + this.parseWhere(where) + ')';

            if (i !== arr.length - 1)
                sql += ' ' + group[0] + ' ';
        });

        return sql;
    }
}

export function AND(...where: Where[]): WhereGroup { return ['AND', where] }
export function OR(...where: Where[]):  WhereGroup { return ['OR',  where] }