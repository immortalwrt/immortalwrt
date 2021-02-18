'use strict';
'require form';
'require fs';
'require uci';

function renameFile(str, tag) {
	var dir = dirName(str);
	var n = str.lastIndexOf('/'), fn = n > -1 ? str.slice(n + 1) : str;
	var n = fn.lastIndexOf('.'), bn = n > -1 ? fn.slice(0, n) : fn;
	var n = fn.lastIndexOf('.'), en = n > -1 ? fn.slice(n + 1) : '';
	return dir + bn + '.' + tag + (en ? '.' + en : '');
}

function dirName(str) {
	var n = str.lastIndexOf('/');
	return n > -1 ? str.slice(0, n + 1) : '';
}

return L.view.extend({
	lastPath: null,

	load: function() {
		return uci.load('wrtbwmon').then(L.bind(function() {
			this.lastPath = uci.get_first('wrtbwmon', 'wrtbwmon', 'path') || null;
		}, this));
	},

	render: function() {
		var m, s, o;

		m = new form.Map('wrtbwmon', _('Usage - Configuration'));

		s = m.section(form.NamedSection, 'general', 'wrtbwmon', _('General settings'));
		s.addremove = false;

		o = s.option(form.Flag, 'enabled', _('Keep running in the background'));
		o.rmempty = true;

		o = s.option(form.Value, 'path', _('Database path'), _('This box is used to select the Database path, which is /tmp/usage.db by default.'));
		o.value('/tmp/usage.db');
		o.value('/etc/usage.db');
		o.default = '/tmp/usage.db';
		o.rmempty = false;

		return m.render();
	},

	changePath: function() {
		return uci.changes().then(L.bind(function(res) {
			if (res.wrtbwmon && this.lastPath) {
				for (var i = 0; i < res.wrtbwmon.length; i++) {
					if (res.wrtbwmon[i][2] == "path") {
						var newPath = res.wrtbwmon[i][3];
						return fs.stat(dirName(newPath)).then(L.bind(function(res) {
							if (res.type == 'directory') {
								Promise.all([
									fs.exec('/bin/cp', ['-fp', this.lastPath, newPath]),
									fs.exec('/bin/cp', ['-fp', renameFile(this.lastPath, '6'), renameFile(newPath, '6')]),
									fs.exec('/bin/cp', ['-fp', renameFile(this.lastPath, '46'), renameFile(newPath, '46')])
								]);
								return true;
							}
							else {
								var err = new Error('Can\'t move files to non-directory path.');
								err.name = 'NotDirectoryError';
								throw err;
							}
						}, this)).catch(function(err) {
							throw err;
						})
					}
				}
			}
			return false;
		}, this));
	},

	handleSaveApply: function(ev, mode) {
		return this.handleSave(ev).then(L.bind(this.changePath, this)).then(L.bind(function(data) {
			L.resolveDefault(L.ui.changes.apply(mode == '0')).then(L.bind(function() {
				if (data) {
					Promise.all([
						fs.exec('/bin/rm', ['-f', this.lastPath]),
						fs.exec('/bin/rm', ['-f', renameFile(this.lastPath, '6')]),
						fs.exec('/bin/rm', ['-f', renameFile(this.lastPath, '46')])
					]);
				}
			}, this));
		}, this)).catch(function(err) {
			if (confirm(err + '\n\n' + _('This will revert the changes. Are you sure?'))) {
				L.bind(L.ui.changes.revert, L.ui.changes)();
			}
		});
	}
});
