# Changelog

## 1.0.13

- Fixed error-causing attempt to convert `null` to camel-case object in `Util.objToCamelCase()` when using `Db.Select()`.

## 1.0.12

- Fixed incorrect `null` -> `"null"` conversion when using entities.

## 1.0.11

- Fixed incorrect default conversion from entity property "null-like" values to database values.

## 1.0.10

- Added `pluck` option when selecting data (`false` by default).

## 1.0.9

- Added `preGet()` and `postGet()` to perform additional initialization operations with entity once it is retreived from database with `<Entity>.getById()` or `<Entity>.from()`.