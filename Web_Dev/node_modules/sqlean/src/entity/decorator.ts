import { FuncFromDb, FuncToDb } from "../Db";
import { EntityData, Entity, ColumnInfo } from "./Entity";

export function Table(tableName: string)
{
    return (constructor) =>
    {
        EntityData.getFrom(constructor).table = tableName;
    }
}

export function PrimaryKey(target: Entity, key: string)
{
    EntityData.getFrom(target.constructor).primaryKey.push(key);
}

export function Column(target: Entity, key: string)
{
    EntityData.addColumn(target, { key: key });
}

export function ColumnCustom(funcs: { toDb?: FuncToDb, fromDb?: FuncFromDb } = {})
{
    return (target: Entity, key: string) =>
    {
        let column = new ColumnInfo;
            column.key = key;
        
        if (funcs.toDb)     column.toDb = funcs.toDb;
        if (funcs.fromDb)   column.fromDb = funcs.fromDb;

        EntityData.addColumn(target, column);
    }
}