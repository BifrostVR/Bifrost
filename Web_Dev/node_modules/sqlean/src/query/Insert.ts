import { ColumnValue, DB } from "../Db";
import { Util } from "../Util";

export class InsertData
{
    table:          string;
    columnValues:   ColumnValue;

    rewrite? =      false;
}

export function Insert(data: InsertData)
{
    data = {...new InsertData, ...data};

    let keys = Object.keys(data.columnValues);

    let dbNameKeys = keys.map(key => Util.toSnakeCase(key));
    let dbValueKeys = keys.map(key => '@' + key);

    let sql = `
        INSERT ${data.rewrite ? 'OR REPLACE' : ''}
        INTO ${data.table} (${dbNameKeys.join(', ')})
        VALUES (${dbValueKeys.join(', ')})
    `;

    // For Debug
    //console.log(sql);

    DB._db.prepare(sql).run(data.columnValues);
}