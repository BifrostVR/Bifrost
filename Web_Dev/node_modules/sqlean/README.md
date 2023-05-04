<div align="center">
  <p></p>
  <a href="https://github.com/CMTV/sqlean">
    <img height="70" src="sqlean.svg">
  </a>
  <p></p>
  <h1>SQLean<p></p></h1>
</div>

<p></p>

SQLean (*/s_clean/*) is a simple SQLite database manager built on top of [better-sqlite3](https://www.npmjs.com/package/better-sqlite3) package. It's main purpose is to allow you to focus on writing **code** rather then messing with **queries**.

Working with a database on a low level is a nightmare since you have to convert your data to database format, then to query strings and vice verca.

With SQLean you can easily perform basic operations with database (select, insert, update, delete) passing data as objects and constructing `WHERE` clauses with code constructions instead of mixing strings with data.

SQLean also supports **entity system** allowing you to create relations between TypeScript classes with SQLite columns using modern decorator syntax. With this system you can forget about SQL-stuff at all and work directly with classes and instances, letting SQLean to do all insert, update, select, delete job behind the scenes.

Before you start:

* SQLean is intended to be used in TypeScript projects
* SQLean is synchronous (just as [better-sqlite3](https://www.npmjs.com/package/better-sqlite3))
* SQLean does not protect you from SQL-injections
* SQLean **always** automatically converts `camelCase` (code side) to `snake_case` (db side) and vice verca

## Quick links

1. [Install](#install)
2. [Opening database](#opening-database)
3. [Simple Queries](#simple-queries)
    * [Insert](#insert)
    * [Building `WHERE` clauses](#building-where-clauses)
    * [Select](#select)
    * [Update and Delete](#update-and-delete)
4. [Entity system](#entity-system)
    * [Creating entity](#creating-entity)
    * [Saving and Updating entity](#saving-and-updating-entity)
    * [Retrieving entity](#retrieving-entity)
    * [Deleting entity](#deleting-entity)
5. [Using better-sqlite3](#using-better-sqlite3)

## Install

```console
$ npm i sqlean
```

## Opening database

Let's assume you already have a SQLite database called `data.db` file with columns structure and maybe some data. A typical way  to create such databases is to use something like [DB4S](https://sqlitebrowser.org/).

First of all, we need to open the database file. It must be done before calling any other SQLean methods!

```typescript
import { Db } from "sqlean";

Db.Open('data.db');

// Do stuff with database
// ...

Db.Close(); // Optional. Can be omitted
```

## Simple Queries

The basic idea is simple. You just call a desired static method of `Db` class and pass data object which contains info about table, columns and where clauses.

### Insert

```typescript
let customerData = {
    customerId: 'cmtv',
    firstName: 'Peter',
    //secondName: 'Radko' // Allowed to be null in database
    age: 23,
    isPro: 1 // SQLite does not support Boolean values
};

Db.Insert({
    table: 'customer',
    columnValues: customerData
});
```

| customer_id | first_name | second_name | age | is_pro |
|-------------|------------|-------------|-----|--------|
| cmtv        | Peter      | *NULL*      | 23  | 1      |

### Building `WHERE` clauses

There are three ways to build `WHERE` clauses:

1 - Pure string clause:
```typescript
let where = `@firstName = 'Peter' AND 5 = 2`;

// first_name = 'Peter' AND 5 = 2
```

2 - Single array-clause:
```typescript
let where: WhereClause;
    
    where = ['@firstName', '=', 'Peter'];
            // WHERE first_name = $param0

    where = [2, '>=', 5];
            // $param0 >= $param1

    where = ['@customerId', 'LIKE', 'c%'];
            // customer_id LIKE $param0
```

3 - Group clauses in which you can combine all three ways:
```typescript
let where = AND(
    ['@firstName', '=', 'Peter'],
    OR(
        ['@customerId', 'LIKE', 'c%'],
        '2 >= 5'
    )
);

// (first_name = $param0) AND ((customer_id LIKE $param1) OR (2 >= 5))
```

There are two important things you should keep in mind.

First of all, everything prefixed with `@` will be converted to `snake_case` and left in query without quotes.

Secondly, notice how strings and numbers are converted into `$paramX` references in array-clauses.
When query run, the values will be automatically injected adding quotes for strings if needed. This works only in array-clauses.

### Select

Selecting one row:

```typescript
let cmtv = Db.Select.Get({
    table: 'customer',
    columns: '*',
    where: ['@firstName', '=', 'Peter']
});
```

```
{ customerId: 'cmtv', firstName: 'Peter', secondName: null, isPro: 1 }
```

Selecting multiple rows:

```typescript
let rows = Db.Select.All({
    table:      'customer',
    columns:    ['customerId', 'age'],
    where:      OR('2 = 2', [3, '>', 20])
});
```

```
[
  { customerId: 'nnagibin', age: 25 },
  { customerId: 'cmtv', age: 23 }     
]
```

Notice how came casel column names automatically convert to snake case database column names and vice versa.

Return values can be plucked:

```typescript
Db.Select.All({
    table:      'customer',
    columns:    ['firstName'],
    pluck:      true
});
```

```
[ 'Nicholas', 'Peter' ]
```

### Update and Delete

The `UPDATE` and `DELETE` queries are built the same way we already discovered:

```typescript
Db.Update({
    table:          'customer',
    columnValues:   { firstName: 'Steve'},
    where:          ['@customerId', '=', 'cmtv']
});

Db.Delete({
    table:  'customer',
    where:  ['@customerId', '=', 'cmtv']
});
```

## Entity system

The Entity system utilizes JS decorators and since it is an experimental feature you need to enable them in `tsconfig.json`.
Moreover you also have to enable a feature for correct type casting from db values to class properties. 

```json
"compilerOptions": {
    "experimentalDecorators": true,
    "emitDecoratorMetadata": true
}
```

### Creating entity

The entity is just a class extending `Entity` abstract class with some decorators on class and properties:

```typescript
import { Column, Entity, PrimaryKey, Table } from "sqlean"

@Table('customer')
class Customer extends Entity 
{
    @PrimaryKey
    @Column
    customerId: string;

    @Column
    firstName: string;

    @Column
    secondName: string;

    @Column
    age: number;

    @Column
    isPro: boolean;
}
```

You can also override `pre/postSave/Delete/Get` methods to run additional queries when entity gets saved, deleted or retrieved from database.

### Saving and Updating entity

Both saving (inserting) and updating entity is done via `save()` method. Calling it cause updating database values. Keep in mind that changing primary key columns will result in creating a new row in column when saving!

```typescript
// Creating entity instance

let cmtv = new Customer;
    cmtv.customerId = 'CMTV';
    cmtv.firstName = 'Peter';
 // cmtv.secondName = 'Radko' // `second_name` column allowed to be null  
    cmtv.age = 23;
    cmtv.isPro = true;

// Inserting entity to database

cmtv.save();
```

| customer_id | first_name | second_name | age | is_pro |
|-------------|------------|-------------|-----|--------|
| CMTV        | Peter      | *NULL*      | 23  | 1      |

```typescript
// Updating entity

cmtv.secondName = 'Radko';
cmtv.isPro = false;

cmtv.save();
```

| customer_id | first_name | second_name | age | is_pro |
|-------------|------------|-------------|-----|--------|
| CMTV        | Peter      | Radko       | 23  | 0      |


### Retrieving entity

Use `getById()` static method of your entity class to retrieve rows from database. The return value is an instance of entity class or `null` if no rows were found.

First argument can be a simple value for single primary key or an array of values for multi-column primary keys.
Optional second argument accepts column names to retrieve in case you don't wont to load heavy data from database.

```typescript
let cmtv = Customer.getById('CMTV');

let shortCmtv = Custom.getById('CMTV', ['customerId', 'age']);
```

```
Customer {
  customerId: 'CMTV', 
  firstName: 'Peter', 
  secondName: 'Radko',
  age: 23,
  isPro: 0
}

Customer { customerId: 'CMTV', age: 23 }
```

### Deleting entity

There are two ways to delete an entity: through its class or directly from instance:

```typescript
Customer.delete('CMTV');

/* OR */

let cmtv = Customer.getById('CMTV');
cmtv.delete();
```

## Using better-sqlite3

If you need to build complex SQL queries or access low-level database settings you can always access [better-sqlite3](https://www.npmjs.com/package/better-sqlite3) database instance directly by accessing `_db` property:

```typescript
import { Db, DB } from "sqlean";

Db.Open('data.db');

DB._db.pragma('cache_size = 32000');
DB._db.pragma('journal_mode = WAL');

DB._db.prepare('CREATE TABLE my_table (my_primary_key INTEGER NOT NULL, my_column TEXT, PRIMARY KEY(my_primary_key))').run();
```

Notice the small difference between `Db` class which is used to access SQLean static methods and `DB` singleton instance!

For more information about `_db` see official better-sqlite3 [docs](https://github.com/JoshuaWise/better-sqlite3/blob/master/docs/api.md).