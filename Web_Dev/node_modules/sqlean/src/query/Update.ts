import { ColumnValue, DB } from "../Db";
import { Util } from "../Util";
import { Where, WhereBuilder } from "./Where";

export class UpdateData
{
    table:          string;
    columnValues:   ColumnValue;
    where?:         Where;
}

export function Update(data: UpdateData)
{
    data = {...new UpdateData, ...data};

    let sqlSet = '';
    let updateValues = {};

    Object.keys(data.columnValues).forEach((key, i, arr) =>
    {
        let updateKey = '__update_' + key;

        sqlSet += `${Util.toSnakeCase(key)} = @${updateKey} ${i !== arr.length - 1 ? ',' : ''}`;
        updateValues[updateKey] = data.columnValues[key];
    });

    let whereResults = new WhereBuilder(data.where || '').getResult();

    let sql = `
        UPDATE ${data.table}
        SET ${sqlSet}
        ${whereResults.sql}
    `;

    // For Debug
    //console.log(sql);

    DB._db.prepare(sql).run({...updateValues, ...whereResults.params});
}