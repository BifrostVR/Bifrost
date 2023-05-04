import "reflect-metadata";

import { Db, DbValue, FuncFromDb, FuncToDb } from "../Db";
import { SelectColumns } from "../query/Select";
import { AND, WhereClause } from "../query/Where";
import { Util } from "../Util";

export type EntityId = DbValue | DbValue[];

export class ColumnInfo
{
    key:        string;
    Type?;
    toDb?:      FuncToDb = Util.defaultToDb;
    fromDb?:    FuncFromDb = Util.defaultFromDb;
}

export class EntityData
{
    table:      string;
    primaryKey: string[] = [];
    columns:    { [column: string]: ColumnInfo } = {};

    static getFrom(constructor): EntityData
    {
        if (!constructor.entityData)
            constructor.entityData = new EntityData;

        return constructor.entityData;
    }

    static addColumn(target, column: ColumnInfo)
    {
        column = {...new ColumnInfo, ...column};
        column.Type = Reflect.getMetadata('design:type', target, column.key);
        EntityData.getFrom(target.constructor).columns[column.key] = column;
    }
}

export abstract class Entity
{
    static entityData;

    protected preSave()     {};
    protected postSave()    {};

    protected preDelete()   {};
    protected postDelete()  {};

    protected preGet()      {};
    protected postGet()     {};

    //

    save()
    {
        this.preSave();

        let entityData = EntityData.getFrom(this.constructor);
        let toInsert = {};

        Object.keys(entityData.columns).forEach(columnKey =>
        {
            let column = entityData.columns[columnKey];
            toInsert[columnKey] = column.toDb(this[columnKey]);
        });

        Db.Insert({
            table: entityData.table,
            columnValues: toInsert,
            rewrite: true
        });

        this.postSave();
    }

    delete()
    {
        this.preDelete();

        let entityData = EntityData.getFrom(this.constructor);
        let clauses: WhereClause[] = entityData.primaryKey.map( pKey => ['@' + pKey, '=', entityData.columns[pKey].toDb(this[pKey])] );
        let where = AND(...clauses);

        Db.Delete({
            table: entityData.table,
            where: where
        });

        this.postDelete();
    }

    //
    //
    //

    static buildWhereId(id: EntityId)
    {
        let entityData = EntityData.getFrom(this.prototype.constructor);
        if (!Array.isArray(id)) id = [id];

        let clauses: WhereClause[] = entityData.primaryKey.map( (pKey, i) => ['@' + pKey, '=', entityData.columns[pKey].toDb(id[i])] );

        return AND(...clauses);
    }

    static delete(id: EntityId)
    {
        let entityData = EntityData.getFrom(this.prototype.constructor);
        //@ts-ignore
        let whereId = this.buildWhereId(id);

        Db.Delete({
            table: entityData.table,
            where: whereId
        });
    }

    static getById<T>(this: new () => T, id: EntityId, columns: SelectColumns = '*'): T
    {
        let entityData = EntityData.getFrom(this.prototype.constructor);
        //@ts-ignore
        let whereId = this.buildWhereId(id);

        let dbResult = Db.Select.Get({
            table: entityData.table,
            columns: columns,
            where: whereId
        });

        // ACCESSING STATIC METHOD FROM CHILD INSTANCE (FALLBACK TO PARENT)
        //@ts-ignore
        return this.from(dbResult);
    }

    static from<T>(this: new () => T, dbResult): T
    {
        if (!dbResult) return null;

        let entity = new this();

        //@ts-ignore
        entity.preGet();

        Object.keys(dbResult).forEach(columnKey =>
        {
            // ACCESSING STATIC PROPERTY FROM CHILD INSTANCE (FALLBACK TO PARENT)
            //@ts-ignore
            let columnInfo = this.entityData.columns[columnKey] as ColumnInfo;
            let dbValue = dbResult[columnKey];

            let value = columnInfo.fromDb(dbValue, columnInfo.Type);

            entity[columnKey] = value;
        });

        //@ts-ignore
        entity.postGet();

        return entity;
    }
}