var gyp = require('pangyp');
var Gyp = new gyp.Gyp();
argv = ['--nodedir=/usr/local/include/node', '-silly', '--libsass_ext=auto'];
Gyp.parseArgv(argv);
Gyp.opts.loglevel='silly'
Gyp.commands.list('', function(a, b){console.log(a,b);});
Gyp.commands.configure([], function(e) {
  console.log(Gyp)
});
