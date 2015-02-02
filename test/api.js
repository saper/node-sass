var assert = require('assert'),
    fs = require('fs'),
    path = require('path'),
    read = fs.readFileSync,
    sass = process.env.NODESASS_COV ? require('../lib-cov') : require('../lib'),
    fixture = path.join.bind(null, __dirname, 'fixtures'),
    resolveFixture = path.resolve.bind(null, __dirname, 'fixtures');

describe('api', function() {
  describe('.render(options)', function() {
    it('should compile sass to css with file', function(done) {
      var expected = read(fixture('simple/expected.css'), 'utf8').trim();

      sass.render({
        file: fixture('simple/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should compile sass to css with data', function(done) {
      var src = read(fixture('simple/index.scss'), 'utf8');
      var expected = read(fixture('simple/expected.css'), 'utf8').trim();

      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should compile sass to css using indented syntax', function(done) {
      var src = read(fixture('indent/index.sass'), 'utf8');
      var expected = read(fixture('indent/expected.css'), 'utf8').trim();

      sass.render({
        data: src,
        indentedSyntax: true,
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should throw error status 1 for bad input', function(done) {
      sass.render({
        data: '#navbar width 80%;',
        error: function(error) {
          assert(error.message);
          assert.equal(error.status, 1);
          done();
        }
      });
    });

    it('should compile with include paths', function(done) {
      var src = read(fixture('include-path/index.scss'), 'utf8');
      var expected = read(fixture('include-path/expected.css'), 'utf8').trim();

      sass.render({
        data: src,
        includePaths: [
          fixture('include-path/functions'),
          fixture('include-path/lib')
        ],
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should compile with image path', function(done) {
      var src = read(fixture('image-path/index.scss'), 'utf8');
      var expected = read(fixture('image-path/expected.css'), 'utf8').trim();

      sass.render({
        data: src,
        imagePath: '/path/to/images',
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should throw error with non-string image path', function(done) {
      var src = read(fixture('image-path/index.scss'), 'utf8');

      assert.throws(function() {
        sass.render({
          data: src,
          imagePath: ['/path/to/images']
        });
      });

      done();
    });

    it('should render with --precision option', function(done) {
      var src = read(fixture('precision/index.scss'), 'utf8');
      var expected = read(fixture('precision/expected.css'), 'utf8').trim();

      sass.render({
        data: src,
        precision: 10,
        success: function(result) {
          assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
          done();
        }
      });
    });

    it('should contain all included files in stats when data is passed', function(done) {
      var src = read(fixture('include-files/index.scss'), 'utf8');
      var expected = [
        fixture('include-files/bar.scss').replace(/\\/g, '/'),
        fixture('include-files/foo.scss').replace(/\\/g, '/')
      ];

      sass.render({
        data: src,
        includePaths: [fixture('include-files')],
        success: function(result) {
          assert.deepEqual(result.stats.includedFiles, expected);
          done();
        }
      });
    });
  });

  describe('.render(importer)', function() {
    var src = read(fixture('include-files/index.scss'), 'utf8');

    it('should override imports with "data" as input and fires callback with file and contents', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev, done) {
          done({
            file: '/some/other/path.scss',
            contents: 'div {color: yellow;}'
          });
        }
      });
    });

    it('should override imports with "file" as input and fires callback with file and contents', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev, done) {
          done({
            file: '/some/other/path.scss',
            contents: 'div {color: yellow;}'
          });
        }
      });
    });

    it('should override imports with "data" as input and returns file and contents', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev) {
          return {
            file: prev + url,
            contents: 'div {color: yellow;}'
          };
        }
      });
    });

    it('should override imports with "file" as input and returns file and contents', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev) {
          return {
            file: prev + url,
            contents: 'div {color: yellow;}'
          };
        }
      });
    });

    it('should override imports with "data" as input and fires callback with file', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), '');
          done();
        },
        importer: function(url, /* jshint unused:false */ prev, done) {
          done({
            file: path.resolve(path.dirname(fixture('include-files/index.scss')), url + (path.extname(url) ? '' : '.scss'))
          });
        }
      });
    });

    it('should override imports with "file" as input and fires callback with file', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), '');
          done();
        },
        importer: function(url, prev, done) {
          done({
            file: path.resolve(path.dirname(prev), url + (path.extname(url) ? '' : '.scss'))
          });
        }
      });
    });

    it('should override imports with "data" as input and returns file', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), '');
          done();
        },
        importer: function(url, /* jshint unused:false */ prev) {
          return {
            file: path.resolve(path.dirname(fixture('include-files/index.scss')), url + (path.extname(url) ? '' : '.scss'))
          };
        }
      });
    });

    it('should override imports with "file" as input and returns file', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), '');
          done();
        },
        importer: function(url, prev) {
          return {
            file: path.resolve(path.dirname(prev), url + (path.extname(url) ? '' : '.scss'))
          };
        }
      });
    });
	
    it('should override imports with "data" as input and fires callback with contents', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev, done) {
          done({
            contents: 'div {color: yellow;}'
          });
        }
      });
    });

    it('should override imports with "file" as input and fires callback with contents', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function(url, prev, done) {
          done({
            contents: 'div {color: yellow;}'
          });
        }
      });
    });

    it('should override imports with "data" as input and returns contents', function(done) {
      sass.render({
        data: src,
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function() {
          return {
            contents: 'div {color: yellow;}'
          };
        }
      });
    });

    it('should override imports with "file" as input and returns contents', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
          done();
        },
        importer: function() {
          return {
            contents: 'div {color: yellow;}'
          };
        }
      });
    });
  });

  describe('.render(functions)', function() {
    it('should call custom defined nullary function', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: 42px; }');
          done();
        },
        functions: {
          'foo()': function() {
            return new sass.types.Number(42, 'px');
          }
        }
      });
    });

    it('should call custom function with multiple args', function(done) {
      sass.render({
        data: 'div { color: foo(3, 42px); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: 126px; }');
          done();
        },
        functions: {
          'foo($a, $b)': function(factor, size) {
            return new sass.types.Number(factor.getValue() * size.getValue(), size.getUnit());
          }
        }
      });
    });

    it('should work with custom functions that return data asynchronously', function(done) {
      sass.render({
        data: 'div { color: foo(42px); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: 66em; }');
          done();
        },
        functions: {
          'foo($a)': function(size, done) {
            setTimeout(function() {
              done(new sass.types.Number(66, 'em'));
            }, 50);
          }
        }
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (number)', function(done) {
      sass.render({
        data: 'div { width: foo(42px); height: bar(42px); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  width: 42rem;\n  height: 84px; }');
          done();
        },
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
      });
    });

    it('should properly convert strings when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: foo("bar"); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: "barbar"; }');
          done();
        },
        functions: {
          'foo($a)': function(str) {
            str = str.getValue().replace(/['"]/g, '');
            return new sass.types.String('"' + str + str + '"');
          }
        }
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (string)', function(done) {
      sass.render({
        data: 'div { width: foo("bar"); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  width: "barbar"; }');
          done();
        },
        functions: {
          'foo($a)': function(str) {
            var unquoted = str.getValue().replace(/['"]/g, '');
            str.setValue('"' + unquoted + unquoted + '"'); 
            return str;
          }
        }
      });
    });

    it('should properly convert colors when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(#f00); background-color: bar(); border-color: baz(); }',
        success: function(result) {
          assert.equal(
            result.css.trim(), 
            'div {\n  color: rgba(255, 255, 0, 0.5);' + 
            '\n  background-color: rgba(255, 0, 255, 0.2);' +
            '\n  border-color: red; }'
          );
          done();
        },
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
      });
    });

    it('should properly convert boolean when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: if(foo(true, false), #fff, #000);' + 
          '\n  background-color: if(foo(true, true), #fff, #000); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: #000;\n  background-color: #fff; }');
          done();
        },
        functions: {
          'foo($a, $b)': function(a, b) {
            return new sass.types.Boolean(a.getValue() && b.getValue());
          }
        }
      });
    });

    it('should let custom functions call setter methods on wrapped sass values (boolean)', function(done) {
      sass.render({
        data: 'div { color: if(foo(false), #fff, #000); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: #fff; }');
          done();
        },
        functions: {
          'foo($a)': function(a) {
            a.setValue(true);
            return a;
          }
        }
      });
    });

    it('should properly convert lists when calling custom functions', function(done) {
      sass.render({
        data: '$test-list: (bar, #f00, 123em); @each $item in foo($test-list) { .#{$item} { color: #fff; } }',
        success: function(result) {
          assert.equal(
            result.css.trim(), 
            '.foo {\n  color: #fff; }\n\n.bar {\n  color: #fff; }\n\n.baz {\n  color: #fff; }'
          );
          done();
        },
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
      });
    });

    it('should properly convert maps when calling custom functions', function(done) {
      sass.render({
        data: '$test-map: foo((abc: 123, #def: true)); div { color: if(map-has-key($test-map, hello), #fff, #000); }' +
          'span { color: map-get($test-map, baz); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: #fff; }\n\nspan {\n  color: qux; }');
          done();
        },
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
      });
    });

    it('should properly convert null when calling custom functions', function(done) {
      sass.render({
        data: 'div { color: if(foo("bar"), #fff, #000); } ' + 
          'span { color: if(foo(null), #fff, #000); }' +
          'table { color: if(bar() == null, #fff, #000); }',
        success: function(result) {
          assert.equal(
            result.css.trim(), 
            'div {\n  color: #000; }\n\nspan {\n  color: #fff; }\n\ntable {\n  color: #fff; }'
          );
          done();
        },
        functions: {
          'foo($a)': function(a) {
            return new sass.types.Boolean(a instanceof sass.types.Null);
          },
          'bar()': function() {
            return new sass.types.Null();
          }
        }
      });
    });

    it('should let us register custom functions without signatures', function(done) {
      sass.render({
        data: 'div { color: foo(20, 22); }',
        success: function(result) {
          assert.equal(result.css.trim(), 'div {\n  color: 42em; }');
          done();
        },
        functions: {
          foo: function(a, b) {
            return new sass.types.Number(a.getValue() + b.getValue(), 'em');
          }
        }
      });
    });

    it('should fail when returning anything other than a sass value from a custom function', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        error: function(e) {
          assert.ok(/A SassValue object was expected/.test(e.message));
          done();
        },
        functions: {
          'foo()': function() {
            return {};
          }
        }
      });
    });

    it('should properly bubble up standard JS errors thrown by custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        error: function(e) {
          assert.ok(/This is a test error/.test(e.message));
          done();
        },
        functions: {
          'foo()': function() {
            throw new RangeError('This is a test error');
          }
        }
      });
    });

    it('should properly bubble up unknown errors thrown by custom functions', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        error: function(e) {
          assert.ok(/unexpected error/.test(e.message));
          done();
        },
        functions: {
          'foo()': function() {
            throw {};
          }
        }
      });
    });

    it('should properly bubble up errors from sass value constructors', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        error: function(e) {
          assert.ok(/Argument should be a bool/.test(e.message));
          done();
        },
        functions: {
          'foo()': function() {
            return new sass.types.Boolean('foo');
          }
        }
      });
    });

    it('should properly bubble up errors from sass value setters', function(done) {
      sass.render({
        data: 'div { color: foo(); }',
        error: function(e) {
          assert.ok(/Supplied value should be a string/.test(e.message));
          done();
        },
        functions: {
          'foo()': function() {
            var ret = new sass.types.Number(42);
            ret.setUnit(123);
            return ret;
          }
        }
      });
    });
  });

  describe('.renderSync(functions)', function() {
    it('should call custom function in sync mode', function(done) {
      var result = sass.renderSync({
        data: 'div { width: cos(0) * 50px; }',
        functions: {
          'cos($a)': function(angle) {
            if (!(angle instanceof sass.types.Number)) {
              throw new TypeError('Unexpected type for "angle"');
            }
            return new sass.types.Number(Math.cos(angle.getValue()));
          }
        }
      });

      assert.equal(result.css.trim(), 'div {\n  width: 50px; }');
      done();
    });

    it('should return a list of selectors after calling the headings custom function', function(done) {
      var result = sass.renderSync({
        data: '#{headings(2,5)} { color: #08c; }',
        functions: {
          'headings($from: 0, $to: 6)': function(from, to) {
            var i, f = from.getValue(), t = to.getValue(),
                list = new sass.types.List(t - f + 1);

            for (i = f; i <= t; i++) {
              list.setValue(i - f, new sass.types.String('h' + i));
            }

            return list;
          }
        }
      });

      assert.equal(result.css.trim(), 'h2, h3, h4, h5 {\n  color: #08c; }');
      done();
    });

    it('should let custom function invoke sass types constructors without the `new` keyword', function(done) {
      var result = sass.renderSync({
        data: 'div { color: foo(); }',
        functions: {
          'foo()': function() {
            return sass.types.Number(42, 'em');
          }
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: 42em; }');
      done();
    });

    it('should let us register custom functions without signatures', function(done) {
      var result = sass.renderSync({
        data: 'div { color: foo(20, 22); }',
        functions: {
          foo: function(a, b) {
            return new sass.types.Number(a.getValue() + b.getValue(), 'em');
          }
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: 42em; }');
      done();
    });

    it('should fail when returning anything other than a sass value from a custom function', function(done) {
      assert.throws(function() {
        sass.renderSync({
          data: 'div { color: foo(); }',
          functions: {
            'foo()': function() {
              return {};
            }
          }
        });
      }, /A SassValue object was expected/);

      done();
    });

    it('should properly bubble up standard JS errors thrown by custom functions', function(done) {
      assert.throws(function() {
        sass.renderSync({
          data: 'div { color: foo(); }',
          functions: {
            'foo()': function() {
              throw new RangeError('This is a test error');
            }
          }
        });
      }, /This is a test error/);
      
      done();
    });

    it('should properly bubble up unknown errors thrown by custom functions', function(done) {
      assert.throws(function() {
        sass.renderSync({
          data: 'div { color: foo(); }',
          functions: {
            'foo()': function() {
              throw {};
            }
          }
        });
      }, /unexpected error/);
      
      done();
    });

    it('should properly bubble up errors from sass value getters/setters/constructors', function(done) {
      assert.throws(function() {
        sass.renderSync({
          data: 'div { color: foo(); }',
          functions: {
            'foo()': function() {
              return new sass.types.Boolean('foo');
            }
          }
        });
      }, /Argument should be a bool/);

      assert.throws(function() {
        sass.renderSync({
          data: 'div { color: foo(); }',
          functions: {
            'foo()': function() {
              var ret = new sass.types.Number(42);
              ret.setUnit(123);
              return ret;
            }
          }
        });
      }, /Supplied value should be a string/);
      
      done();
    });
  });

  describe('.renderSync(options)', function() {
    it('should compile sass to css with file', function(done) {
      var expected = read(fixture('simple/expected.css'), 'utf8').trim();
      var result = sass.renderSync({file: fixture('simple/index.scss')});

      assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
      done();
    });

    it('should compile sass to css with data', function(done) {
      var src = read(fixture('simple/index.scss'), 'utf8');
      var expected = read(fixture('simple/expected.css'), 'utf8').trim();
      var result = sass.renderSync({data: src});

      assert.equal(result.css.trim(), expected.replace(/\r\n/g, '\n'));
      done();
    });

    it('should compile sass to css using indented syntax', function(done) {
      var src = read(fixture('indent/index.sass'), 'utf8');
      var expected = read(fixture('indent/expected.css'), 'utf8').trim();
      var css = sass.renderSync({
        data: src,
        indentedSyntax: true
      }).css.trim();

      assert.equal(css, expected.replace(/\r\n/g, '\n'));
      done();
    });

    it('should throw error for bad input', function(done) {
      assert.throws(function() {
        sass.renderSync({data: '#navbar width 80%;'});
      });

      done();
    });
  });

  describe('.renderSync(importer)', function() {
    var src = read(fixture('include-files/index.scss'), 'utf8');

    it('should override imports with "data" as input and returns file and contents', function(done) {
      var result = sass.renderSync({
        data: src,
        importer: function(url, prev) {
          return {
            file: prev + url,
            contents: 'div {color: yellow;}'
          };
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
      done();
    });

    it('should override imports with "file" as input and returns file and contents', function(done) {
      var result = sass.renderSync({
        file: fixture('include-files/index.scss'),
        importer: function(url, prev) {
          return {
            file: prev + url,
            contents: 'div {color: yellow;}'
          };
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
      done();
    });

    it('should override imports with "data" as input and returns file', function(done) {
      var result = sass.renderSync({
        data: src,
        importer: function(url, /* jshint unused:false */ prev) {
          return {
            file: path.resolve(path.dirname(fixture('include-files/index.scss')), url + (path.extname(url) ? '' : '.scss'))
          };
        }
      });

      assert.equal(result.css.trim(), '');
      done();
    });

    it('should override imports with "file" as input and returns file', function(done) {
      var result = sass.renderSync({
        file: fixture('include-files/index.scss'),
        importer: function(url, prev) {
          return {
            file: path.resolve(path.dirname(prev), url + (path.extname(url) ? '' : '.scss'))
          };
        }
      });

      assert.equal(result.css.trim(), '');
      done();
    });
	
    it('should override imports with "data" as input and returns contents', function(done) {
      var result = sass.renderSync({
        data: src,
        importer: function() {
          return {
            contents: 'div {color: yellow;}'
          };
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
      done();
    });

    it('should override imports with "file" as input and returns contents', function(done) {
      var result = sass.renderSync({
        file: fixture('include-files/index.scss'),
        importer: function() {
          return {
            contents: 'div {color: yellow;}'
          };
        }
      });

      assert.equal(result.css.trim(), 'div {\n  color: yellow; }\n\ndiv {\n  color: yellow; }');
      done();
    });
  });

  describe('.render({stats: {}})', function() {
    var start = Date.now();

    it('should provide a start timestamp', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert(typeof result.stats.start === 'number');
          assert(result.stats.start >= start);
          done();
        },
        error: function(err) {
          assert(!err);
          done();
        }
      });
    });

    it('should provide an end timestamp', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert(typeof result.stats.end === 'number');
          assert(result.stats.end >= result.stats.start);
          done();
        },
        error: function(err) {
          assert(!err);
          done();
        }
      });
    });

    it('should provide a duration', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert(typeof result.stats.duration === 'number');
          assert.equal(result.stats.end - result.stats.start, result.stats.duration);
          done();
        },
        error: function(err) {
          assert(!err);
          done();
        }
      });
    });

    it('should contain the given entry file', function(done) {
      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.equal(result.stats.entry, fixture('include-files/index.scss'));
          done();
        },
        error: function(err) {
          assert(!err);
          done();
        }
      });
    });

    it('should contain an array of all included files', function(done) {
      var expected = [
        fixture('include-files/bar.scss').replace(/\\/g, '/'),
        fixture('include-files/foo.scss').replace(/\\/g, '/'),
        fixture('include-files/index.scss').replace(/\\/g, '/')
      ];

      sass.render({
        file: fixture('include-files/index.scss'),
        success: function(result) {
          assert.deepEqual(result.stats.includedFiles, expected);
          done();
        },
        error: function(err) {
          assert(!err);
          done();
        }
      });
    });

    it('should contain array with the entry if there are no import statements', function(done) {
      var expected = fixture('simple/index.scss').replace(/\\/g, '/');

      sass.render({
        file: fixture('simple/index.scss'),
        success: function(result) {
          assert.deepEqual(result.stats.includedFiles, [expected]);
          done();
        }
      });
    });

    it('should state `data` as entry file', function(done) {
      sass.render({
        data: read(fixture('simple/index.scss'), 'utf8'),
        success: function(result) {
          assert.equal(result.stats.entry, 'data');
          done();
        }
      });
    });

    it('should contain an empty array as includedFiles', function(done) {
      sass.render({
        data: read(fixture('simple/index.scss'), 'utf8'),
        success: function(result) {
          assert.deepEqual(result.stats.includedFiles, []);
          done();
        }
      });
    });
  });

  describe('.renderSync({stats: {}})', function() {
    var start = Date.now();
    var result = sass.renderSync({
      file: fixture('include-files/index.scss')
    });

    it('should provide a start timestamp', function(done) {
      assert(typeof result.stats.start === 'number');
      assert(result.stats.start >= start);
      done();
    });

    it('should provide an end timestamp', function(done) {
      assert(typeof result.stats.end === 'number');
      assert(result.stats.end >= result.stats.start);
      done();
    });

    it('should provide a duration', function(done) {
      assert(typeof result.stats.duration === 'number');
      assert.equal(result.stats.end - result.stats.start, result.stats.duration);
      done();
    });

    it('should contain the given entry file', function(done) {
      assert.equal(result.stats.entry, resolveFixture('include-files/index.scss'));
      done();
    });

    it('should contain an array of all included files', function(done) {
      var expected = [
        fixture('include-files/bar.scss').replace(/\\/g, '/'),
        fixture('include-files/foo.scss').replace(/\\/g, '/'),
        fixture('include-files/index.scss').replace(/\\/g, '/')
      ];

      assert.equal(result.stats.includedFiles[0], expected[0]);
      assert.equal(result.stats.includedFiles[1], expected[1]);
      assert.equal(result.stats.includedFiles[2], expected[2]);
      done();
    });

    it('should contain array with the entry if there are no import statements', function(done) {
      var expected = fixture('simple/index.scss').replace(/\\/g, '/');

      var result = sass.renderSync({
        file: fixture('simple/index.scss')
      });

      assert.deepEqual(result.stats.includedFiles, [expected]);
      done();
    });

    it('should state `data` as entry file', function(done) {
      var result = sass.renderSync({
        data: read(fixture('simple/index.scss'), 'utf8')
      });

      assert.equal(result.stats.entry, 'data');
      done();
    });

    it('should contain an empty array as includedFiles', function(done) {
      var result = sass.renderSync({
        data: read(fixture('simple/index.scss'), 'utf8')
      });

      assert.deepEqual(result.stats.includedFiles, []);
      done();
    });
  });

  describe('.info()', function() {
    it('should return a correct version info', function(done) {
      assert.equal(sass.info(), [
        'node-sass version: ' + require('../package.json').version, 
        'libsass version: ' + require('../package.json').libsass 
      ].join('\n'));

      done();
    });
  });
});
