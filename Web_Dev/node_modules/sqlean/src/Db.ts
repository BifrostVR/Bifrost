import SQLite from "better-sqlite3";

// Queries
import { Insert } from "./query/Insert";
import { Select } from "./query/Select";
import { Delete } from "./query/Delete";
import { Update } from "./query/Update";

export type FuncToDb =      (src: any) => DbValue;
export type FuncFromDb =    (dbValue: DbValue, Type: any) => any;

export type DbValue = null | number | string;

export type ColumnValue = {
    [column: string]: DbValue
};

export class Db
{
    _db: SQLite.Database;

    static Open(pathToDb: string)   { DB._db = SQLite(pathToDb); }
    static Close()                  { DB._db.close(); }

    static Select = Select;
    static Insert = Insert;
    static Update = Update;
    static Delete = Delete;

    static Transaction(queries: () => void)
    {
        DB._db.transaction(queries)();
    }
}

export const DB = new Db;