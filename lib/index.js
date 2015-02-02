var fs = require('fs'),
    path = require('path');

/**
 * Get binding
 *
 * @api private
 */

function getBinding() {
  var name = process.platform + '-' + process.arch;
  var candidates = [
    path.join(__dirname, '..', 'build', 'Release', 'binding.node'),
    path.join(__dirname, '..', 'build', 'Debug', 'binding.node'),
    path.join(__dirname, '..', 'vendor', name, 'binding.node')
  ];

  var candidate = candidates.filter(fs.existsSync)[0];

  if (!candidate) {
    throw new Error('`libsass` bindings not found. Try reinstalling `node-sass`?');
  }

  return candidate;
}

/**
 * Get outfile
 *
 * @param {Object} options
 * @api private
 */

function getOutFile(options) {
  var file = options.file;
  var outFile = options.outFile;

  if (!outFile || typeof outFile !== 'string' || (!options.data && !file)) {
    return null;
  }

  if (path.resolve(outFile) === path.normalize(outFile).replace(/(.+)([\/|\\])$/, '$1')) {
    return outFile;
  }

  return path.resolve(path.dirname(file), outFile);
}

/**
 * Get stats
 *
 * @param {Object} options
 * @api private
 */

function getStats(options) {
  var stats = {};

  stats.entry = options.file || 'data';
  stats.start = Date.now();

  return stats;
}

/**
 * End stats
 *
 * @param {Object} stats
 * @param {Object} sourceMap
 * @api private
 */

function endStats(stats) {
  stats.end = Date.now();
  stats.duration = stats.end - stats.start;

  return stats;
}

/**
 * Get style
 *
 * @param {Object} options
 * @api private
 */

function getStyle(options) {
  var style = options.output_style || options.outputStyle;
  var styles = {
    nested: 0,
    expanded: 1,
    compact: 2,
    compressed: 3
  };

  return styles[style];
}

/**
 * Get source map
 *
 * @param {Object} options
 * @api private
 */

function getSourceMap(options) {
  var file = options.file;
  var outFile = options.outFile;
  var sourceMap = options.sourceMap;

  if (sourceMap) {
    if (typeof sourceMap !== 'string') {
      sourceMap = outFile ? outFile + '.map' : '';
    } else if (outFile) {
      sourceMap = path.resolve(path.dirname(file), sourceMap);
    }
  }

  return sourceMap;
}

/**
 * Get options
 *
 * @param {Object} options
 * @api private
 */

function getOptions(options) {
  options = options || {};
  options.comments = options.source_comments || options.sourceComments || false;
  options.data = options.data || null;
  options.file = options.file || null;
  options.imagePath = options.image_path || options.imagePath || '';
  options.outFile = getOutFile(options);
  options.paths = (options.include_paths || options.includePaths || []).join(path.delimiter);
  options.precision = parseInt(options.precision) || 5;
  options.sourceMap = getSourceMap(options);
  options.style = getStyle(options) || 0;

  if (options.imagePath && typeof options.imagePath !== 'string') {
    throw new Error('`imagePath` needs to be a string');
  }

  var error = options.error;
  var success = options.success;

  options.error = function(err, code) {
    try {
      err = JSON.parse(err);
    } catch (e) {
      err = { message: err };
    }

    err.code = code;

    if (error) {
      error(err);
    }
  };

  options.success = function() {
    var result = options.result;
    var stats = endStats(result.stats);

    if (success) {
      success({
        css: result.css,
        map: result.sourceMap,
        stats: stats
      });
    }
  };

  delete options.image_path;
  delete options.include_paths;
  delete options.includePaths;
  delete options.source_comments;
  delete options.sourceComments;

  options.result = {
    stats: getStats(options)
  };

  return options;
}

/**
 * Executes a callback and transforms any exception raised into a sass error
 *
 * @param {Function} callback
 * @param {Array} arguments
 * @api private
 */

function tryCallback(callback, args) {
  try {
    return callback.apply(this, args);
  } catch (e) {
    if (typeof e === 'string') {
      return new binding.types.Error(e);
    } else if (e instanceof Error) {
      return new binding.types.Error(e.message);
    } else {
      return new binding.types.Error('An unexpected error occurred');
    }
  }
}

/**
 * Normalizes the signature of custom functions to make it possible to just supply the
 * function name and have the signature default to `fn(...)`. The callback is adjusted
 * to transform the input sass list into discrete arguments.
 *
 * @param {String} signature
 * @param {Function} callback
 * @return {Object}
 * @api private
 */

function normalizeFunctionSignature(signature, callback) {
  if (!/^\*|@warn|@error|@debug|\w+\(.*\)$/.test(signature)) {
    if (!/\w+/.test(signature)) {
      throw new Error('Invalid function signature format "' + signature + '"');
    }

    return {
      signature: signature + '(...)',
      callback: function() {
        var args = Array.prototype.slice.call(arguments),
            list = args.shift(),
            i;

        for (i = list.getLength() - 1; i >= 0; i--) {
          args.unshift(list.getValue(i));
        }

        return callback.apply(this, args);
      }
    };
  }

  return {
    signature: signature, 
    callback: callback
  };
}

/**
 * Require binding
 */

var binding = require(getBinding());

// Expose sass types
module.exports.types = binding.types;

/**
 * Render
 *
 * @param {Object} options
 * @api public
 */

module.exports.render = function(options) {
  options = getOptions(options);

  var importer = options.importer;

  if (importer) {
    options.importer = function(file, prev, bridge) {
      function done(data) {
        bridge.success(data);
      }

      var result = importer(file, prev, done);

      if (result) {
        done(result);
      }
    };
  }

  var functions = options.functions;

  if (functions) {
    options.functions = {};

    Object.keys(functions).forEach(function(signature) {
      var cb = normalizeFunctionSignature(signature, functions[signature]);

      options.functions[cb.signature] = function() {
        var args = Array.prototype.slice.call(arguments),
            bridge = args.pop();

        function done(data) {
          bridge.success(data);
        }

        var result = tryCallback(cb.callback, args.concat(done));

        if (result) {
          done(result);
        }
      };
    });
  }


  options.data ? binding.render(options) : binding.renderFile(options);
};

/**
 * Render sync
 *
 * @param {Object} options
 * @api public
 */

module.exports.renderSync = function(options) {
  options = getOptions(options);

  var functions = options.functions;

  if (options.functions) {
    options.functions = {};

    Object.keys(functions).forEach(function(signature) {
      var cb = normalizeFunctionSignature(signature, functions[signature]);

      options.functions[cb.signature] = function() {
        return tryCallback(cb.callback, arguments);
      };
    });
  }

  var status = options.data ? binding.renderSync(options) : binding.renderFileSync(options);
  var result = options.result;

  if(status) {
    result.stats = endStats(result.stats);

    return result;
  }
};

/**
 * API Info
 *
 */

module.exports.info = function() {
  var package = require('../package.json');

  return [
           'node-sass version: ' + package.version,
           'libsass version: ' + package.libsass
         ].join('\n');
};
