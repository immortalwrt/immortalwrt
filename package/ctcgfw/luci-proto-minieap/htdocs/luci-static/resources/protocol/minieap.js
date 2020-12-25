'use strict';
'require form';
'require network';

network.registerErrorCode('MISSING_USER_OR_PASS', _('Missing username or password'));
network.registerErrorCode('EXIT_FAILURE', _('Program exited with failure. See system log for more information.'));

return network.registerProtocol('minieap', {
	getI18n: function() {
		return _('MiniEAP client');
	},

	getOpkgPackage: function() {
		return 'minieap';
	},

	renderFormOptions: function(s) {
		var dev = this.getL2Device() || this.getDevice(), o;

		// remove firewall tab, add rjv3 tab
		try {
			s.tab('rjv3', _('RJv3 Plugin Settings'), null);
			s.tab_names.splice(2, 0, s.tab_names.pop());
		} catch (e) {}

		o = s.taboption('general', form.Value, 'username', _('Username'));

		o = s.taboption('general', form.Value, 'password', _('Password'));
		o.password = true;

		o = s.taboption('general', form.DynamicList, 'module', _('Packet plugins'));
		o.rmempty = true;
		o.datatype = 'list(string)';
		o.value('rjv3', 'rjv3');
		o.value('printer', 'printer');

		o = s.taboption('advanced', form.Value, 'max_fail', _('Max fail'));
		o.datatype = 'uinteger';
		o.placeholder = '3';

		o = s.taboption('advanced', form.Value, 'max_retries', _('Max retries'));
		o.datatype = 'uinteger';
		o.placeholder = '3';

		o = s.taboption('advanced', form.Flag, 'no_auto_reauth', _('No auto reauth'));

		o = s.taboption('advanced', form.Value, 'wait_after_fail', _('Wait after fail'));
		o.datatype = 'uinteger';
		o.placeholder = '30';

		o = s.taboption('advanced', form.Value, 'stage_timeout', _('Stage timeout'));
		o.datatype = 'uinteger';
		o.placeholder = '5';

		o = s.taboption('advanced', form.Value, 'auth_round', _('Auth Round'));
		o.datatype = 'uinteger';
		o.placeholder = '1';

		o = s.taboption('advanced', form.Value, 'log_file', _('Log file'));
		o.placeholder = '/var/log/minieap.log';

		// rjv3 plugin
		o = s.taboption('rjv3', form.Value, 'heartbeat', _('Heartbeat interval'));
		o.datatype = 'uinteger';
		o.placeholder = '60';

		o = s.taboption('rjv3', form.ListValue, 'eap_bcast_addr', _('EAP broadcast address'));
		o.value(0, _('Standard')); // BROADCAST_STANDARD
		o.value(1, _('RJ private')); // BROADCAST_RJ
		// o.value(2, _('BROADCAST_CER')); // BROADCAST_CER

		o = s.taboption('rjv3', form.ListValue, 'dhcp_type', _('DHCP type'));
		o.value(0, _('Disabled')); // DHCP_NONE
		o.value(1, _('Double Auth')); // DHCP_DOUBLE_AUTH
		o.value(2, _('DHCP After Auth')); // DHCP_AFTER_AUTH
		o.value(3, _('DHCP Before Auth')); // DHCP_BEFORE_AUTH

		o = s.taboption('rjv3', form.DynamicList, 'rj_option', _('RJ option'));
		o.rmempty = true;
		o.placeholder = '<type>:<value>[:r]';

		o = s.taboption('rjv3', form.Value, 'service', _('Service name'));
		o.placeholder = 'internet';

		o = s.taboption('rjv3', form.Value, 'version_str', _('Version string'));
		o.placeholder = 'RG-SU For Linux V1.0';

		o = s.taboption('rjv3', form.Value, 'dhcp_script', _('DHCP script'));

		o = s.taboption('rjv3', form.Value, 'fake_dns1', _('Fake DNS 1'));
		o.datatype = 'ip4addr';

		o = s.taboption('rjv3', form.Value, 'fake_dns2', _('Fake DNS 2'));
		o.datatype = 'ip4addr';

		o = s.taboption('rjv3', form.Value, 'fake_serial', _('Fake HDD serial'));

		o = s.taboption('rjv3', form.Value, 'max_dhcp_count', _('Max DHCP count'));
		o.datatype = 'uinteger';
		o.placeholder = '3';
	}
});
