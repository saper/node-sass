var assert = require('assert'),
    sass = process.env.NODESASS_COV ? require('../lib-cov') : require('../lib');

describe('api', function() {
  describe('.render(functions)', function() {
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
  });
});
