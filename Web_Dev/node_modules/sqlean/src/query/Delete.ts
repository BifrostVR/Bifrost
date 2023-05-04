import { DB } from "../Db";
import { Where, WhereBuilder } from "./Where";

export class DeleteData
{
    table:  string;
    where?: Where;
}

export function Delete(data: DeleteData)
{
    data = {...new DeleteData, ...data};

    let whereResults = new WhereBuilder(data.where || '').getResult();

    let sql = `
        DELETE FROM ${data.table}
        ${whereResults.sql}
    `;

    // For Debug
    //console.log(sql);

    DB._db.prepare(sql).run(whereResults.params);
}