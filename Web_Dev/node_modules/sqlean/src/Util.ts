import { DbValue } from "./Db";

export class Util
{
    static toCamelCase(str: string)
    {
        return str.replace(/_([a-z])/g, (match) => match[1].toUpperCase());
    }

    static objToCamelCase(obj)
    {
        if (obj === null || typeof obj !== 'object') return obj;

        let result = {};
        Object.keys(obj).forEach(key => result[Util.toCamelCase(key)] = obj[key]);

        return result;
    }

    static toSnakeCase(str: string)
    {
        return str.replace(/[A-Z]/g, letter => `_${letter.toLowerCase()}`);
    }

    //
    //
    //

    static defaultToDb(src): DbValue
    {
        if (src === null) return null;

        switch (typeof src)
        {
            case 'string':  return '' + src;

            case 'boolean':
            case 'number':    
                return +src;

            case 'object':  return JSON.stringify(src);

            default: return null;
        }
    }

    static defaultFromDb(dbValue: DbValue, Type)
    {
        if (dbValue === null) return null;

        switch (Type)
        {
            case Number:
                return +dbValue;
            case String:
                return '' + dbValue;
            case Boolean:
                return !!dbValue;
        }

        if (typeof dbValue === 'string')
        {
            try
            {
                let obj = new Type();
                let json = JSON.parse(dbValue);
                
                Object.keys(json).forEach(key => obj[key] = json[key]);

                return obj;
            }
            catch {}
        }

        return dbValue;
    }
}