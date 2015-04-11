var assert = require('assert'),
    sass = process.env.NODESASS_COV ? require('../lib-cov') : require('../lib');

describe('api', function() {
  describe('.render(functions)', function() {
    it('should call custom defined nullary function', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            return new sass.types.Number(42, 'px');
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: 42px; }');
        done();
      });
    });

    it('should call custom function with multiple args', function(done) {
      sass.render({
        data: 'div { color: foo(3, 42px); }',
        functions: {
          'foo($a, $b)': function(factor, size) {
            return new sass.types.Number(factor.getValue() * size.getValue(), size.getUnit());
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: 126px; }');
        done();
      });
    });

    it('should work with custom functions that return data asynchronously', function(done) {
      sass.render({
        data: 'div { color: foo(42px); }',
        functions: {
          'foo($a)': function(size, done) {
            setTimeout(function() {
              done(new sass.types.Number(66, 'em'));
            }, 50);
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: 66em; }');
        done();
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (number)', function(done) {
      sass.render({
        data: 'div { width: foo(42px); height: bar(42px); }',
        functions: {
          'foo($a)': function(size) {
            size.setUnit('rem');
            return size;
          },
          'bar($a)': function(size) {
            size.setValue(size.getValue() * 2);
            return size;
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  width: 42rem;\n  height: 84px; }');
        done();
      });
    });

    it('should properly convert strings when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: foo("bar"); }',
        functions: {
          'foo($a)': function(str) {
            str = str.getValue().replace(/['"]/g, '');
            return new sass.types.String('"' + str + str + '"');
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: "barbar"; }');
        done();
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (string)', function(done) {
      sass.render({
        data: 'div { width: foo("bar"); }',
        functions: {
          'foo($a)': function(str) {
            var unquoted = str.getValue().replace(/['"]/g, '');
            str.setValue('"' + unquoted + unquoted + '"');
            return str;
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  width: "barbar"; }');
        done();
      });
    });

    it('should properly convert colors when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(#f00); background-color: bar(); border-color: baz(); }',
        functions: {
          'foo($a)': function(color) {
            assert.equal(color.getR(), 255);
            assert.equal(color.getG(), 0);
            assert.equal(color.getB(), 0);
            assert.equal(color.getA(), 1.0);

            return new sass.types.Color(255, 255, 0, 0.5);
          },
          'bar()': function() {
            return new sass.types.Color(0x33ff00ff);
          },
          'baz()': function() {
            return new sass.types.Color(0xffff0000);
          }
        }
      }, function(error, result) {
        assert.equal(
          result.css.toString().trim(),
          'div {\n  color: rgba(255, 255, 0, 0.5);' +
          '\n  background-color: rgba(255, 0, 255, 0.2);' +
          '\n  border-color: red; }'
        );
        done();
      });
    });

    it('should properly convert boolean when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: if(foo(true, false), #fff, #000);' +
          '\n  background-color: if(foo(true, true), #fff, #000); }',
        functions: {
          'foo($a, $b)': function(a, b) {
            return sass.types.Boolean(a.getValue() && b.getValue());
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: #000;\n  background-color: #fff; }');
        done();
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (boolean)', function(done) {
      sass.render({
        data: 'div { color: if(foo(false), #fff, #000); background-color: if(foo(true), #fff, #000); }',
        functions: {
          'foo($a)': function(a) {
            return a.getValue() ? sass.types.Boolean.FALSE : sass.types.Boolean.TRUE;
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: #fff;\n  background-color: #000; }');
        done();
      });
    });

    it('should properly convert lists when calling custom functions', function(done) {
      sass.render({
        data: '$test-list: (bar, #f00, 123em); @each $item in foo($test-list) { .#{$item} { color: #fff; } }',
        functions: {
          'foo($l)': function(list) {
            assert.equal(list.getLength(), 3);
            assert.ok(list.getValue(0) instanceof sass.types.String);
            assert.equal(list.getValue(0).getValue(), 'bar');
            assert.ok(list.getValue(1) instanceof sass.types.Color);
            assert.equal(list.getValue(1).getR(), 0xff);
            assert.equal(list.getValue(1).getG(), 0);
            assert.equal(list.getValue(1).getB(), 0);
            assert.ok(list.getValue(2) instanceof sass.types.Number);
            assert.equal(list.getValue(2).getValue(), 123);
            assert.equal(list.getValue(2).getUnit(), 'em');

            var out = new sass.types.List(3);
            out.setValue(0, new sass.types.String('foo'));
            out.setValue(1, new sass.types.String('bar'));
            out.setValue(2, new sass.types.String('baz'));
            return out;
          }
        }
      }, function(error, result) {
        assert.equal(
          result.css.toString().trim(),
          '.foo {\n  color: #fff; }\n\n.bar {\n  color: #fff; }\n\n.baz {\n  color: #fff; }'
        );
        done();
      });
    });

    it('should properly convert maps when calling custom functions', function(done) {
      sass.render({
        data: '$test-map: foo((abc: 123, #def: true)); div { color: if(map-has-key($test-map, hello), #fff, #000); }' +
          'span { color: map-get($test-map, baz); }',
        functions: {
          'foo($m)': function(map) {
            assert.equal(map.getLength(), 2);
            assert.ok(map.getKey(0) instanceof sass.types.String);
            assert.ok(map.getKey(1) instanceof sass.types.Color);
            assert.ok(map.getValue(0) instanceof sass.types.Number);
            assert.ok(map.getValue(1) instanceof sass.types.Boolean);
            assert.equal(map.getKey(0).getValue(), 'abc');
            assert.equal(map.getValue(0).getValue(), 123);
            assert.equal(map.getKey(1).getR(), 0xdd);
            assert.equal(map.getValue(1).getValue(), true);

            var out = new sass.types.Map(3);
            out.setKey(0, new sass.types.String('hello'));
            out.setValue(0, new sass.types.String('world'));
            out.setKey(1, new sass.types.String('foo'));
            out.setValue(1, new sass.types.String('bar'));
            out.setKey(2, new sass.types.String('baz'));
            out.setValue(2, new sass.types.String('qux'));
            return out;
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: #fff; }\n\nspan {\n  color: qux; }');
        done();
      });
    });

    it('should properly convert null when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: if(foo("bar"), #fff, #000); } ' +
          'span { color: if(foo(null), #fff, #000); }' +
          'table { color: if(bar() == null, #fff, #000); }',
        functions: {
          'foo($a)': function(a) {
            return sass.types.Boolean(a instanceof sass.types.Null);
          },
          'bar()': function() {
            return sass.NULL;
          }
        }
      }, function(error, result) {
        assert.equal(
          result.css.toString().trim(),
          'div {\n  color: #000; }\n\nspan {\n  color: #fff; }\n\ntable {\n  color: #fff; }'
        );
        done();
      });
    });

    it('should be possible to carry sass values across different renders', function(done) {
      var persistentMap;

      sass.render({
        data: 'div { color: foo((abc: #112233, #ddeeff: true)); }',
        functions: {
          foo: function(m) {
            persistentMap = m;
            return sass.types.Color(0, 0, 0);
          }
        }
      }, function() {
        sass.render({
          data: 'div { color: map-get(bar(), abc); background-color: baz(); }',
          functions: {
            bar: function() {
              return persistentMap;
            },
            baz: function() {
              return persistentMap.getKey(1);
            }
          }
        }, function(errror, result) {
          assert.equal(result.css.toString().trim(), 'div {\n  color: #112233;\n  background-color: #ddeeff; }');
          done();
        });
      });
    });

    it('should let us register custom functions without signatures', function(done) {
      sass.render({
        data: 'div { color: foo(20, 22); }',
        functions: {
          foo: function(a, b) {
            return new sass.types.Number(a.getValue() + b.getValue(), 'em');
          }
        }
      }, function(error, result) {
        assert.equal(result.css.toString().trim(), 'div {\n  color: 42em; }');
        done();
      });
    });

    it('should fail when returning anything other than a sass value from a custom function', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            return {};
          }
        }
      }, function(error) {
        assert.ok(/A SassValue object was expected/.test(error.message));
        done();
      });
    });

    it('should properly bubble up standard JS errors thrown by custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            throw new RangeError('This is a test error');
          }
        }
      }, function(error) {
        assert.ok(/This is a test error/.test(error.message));
        done();
      });
    });

    it('should properly bubble up unknown errors thrown by custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            throw {};
          }
        }
      }, function(error) {
        assert.ok(/unexpected error/.test(error.message));
        done();
      });
    });

    it('should properly bubble up errors from sass value constructors', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            return sass.types.Boolean('foo');
          }
        }
      }, function(error) {
        assert.ok(/Expected one boolean argument/.test(error.message));
        done();
      });
    });

    it('should properly bubble up errors from sass value setters', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            var ret = new sass.types.Number(42);
            ret.setUnit(123);
            return ret;
          }
        }
      }, function(error) {
        assert.ok(/Supplied value should be a string/.test(error.message));
        done();
      });
    });

    it('should always map null, true and false to the same (immutable) object', function(done) {
      var counter = 0;

      sass.render({
        data: 'div { color: foo(bar(null)); background-color: baz("foo" == "bar"); }',
        functions: {
          foo: function(a) {
            assert.ok(
              'Supplied value should be the same instance as sass.TRUE',
              a === sass.TRUE
            );

            assert.ok(
              'sass.types.Boolean(true) should return a singleton',
              sass.types.Boolean(true) === sass.types.Boolean(true) &&
              sass.types.Boolean(true) === sass.TRUE
            );

            counter++;

            return sass.types.String('foo');
          },
          bar: function(a) {
            assert.ok(
              'Supplied value should be the same instance as sass.NULL',
              a === sass.NULL
            );

            assert.throws(function() {
              return new sass.types.Null();
            }, /Cannot instantiate SassNull/);

            counter++;

            return sass.TRUE;
          },
          baz: function(a) {
            assert.ok(
              'Supplied value should be the same instance as sass.FALSE',
              a === sass.FALSE
            );

            assert.throws(function() {
              return new sass.types.Boolean(false);
            }, /Cannot instantiate SassBoolean/);

            assert.ok(
              'sass.types.Boolean(false) should return a singleton',
              sass.types.Boolean(false) === sass.types.Boolean(false) &&
              sass.types.Boolean(false) === sass.FALSE
            );

            counter++;

            return sass.types.String('baz');
          }
        }
      }, function() {
        assert.ok(counter === 3);
        done();
      });
    });
  });
});
