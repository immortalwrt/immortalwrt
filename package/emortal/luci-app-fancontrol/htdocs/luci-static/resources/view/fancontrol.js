'use strict';
'require view';
'require fs';
'require form';
'require uci';

const THERMAL_FILE_PLACEHOLDER = '/sys/devices/virtual/thermal/thermal_zone0/temp';
const FAN_FILE_PLACEHOLDER = '/sys/devices/platform/pwm-fan/hwmon/hwmon0/pwm1';

async function readFile(filePath) {
	try {
		const rawData = await fs.read_direct(filePath);
		return parseInt(rawData);
	} catch (err) {
		return null; // 返回null表示读取失败
	}
}
//form.DummyValue是文本
return view.extend({
	load: function() {
		return Promise.all([uci.load('fancontrol')]);
	},
	render: async function(data) {
		const m = new form.Map('fancontrol', _('Fan General Control'));

		const s = m.section(form.TypedSection, 'fancontrol', _('Settings'));
		s.anonymous = true;

		// Enabled option
		let o = s.option(form.Flag, 'enable', _('Enable'), _('Enable'));
		o.rmempty = false;

		// Thermal file option
		o = s.option(form.Value, 'thermal_file', _('Thermal File'), _('Thermal File'));
		o.placeholder = THERMAL_FILE_PLACEHOLDER;

		const tempDiv = uci.get('fancontrol', 'settings', 'temp_div');
		const temp = await readFile(uci.get('fancontrol', 'settings', 'thermal_file'));
		if (temp !== null && tempDiv > 0) {
			o.description = _('Current temperature:') + ` <b>${(temp / tempDiv).toFixed(2)}°C</b>`;
		} else {
			o.description = _('Error reading temperature or invalid temp_div');
		}

		// Fan file option
		o = s.option(form.Value, 'fan_file', _('Fan Speed File'), _('Fan Speed File'));
		o.placeholder = FAN_FILE_PLACEHOLDER;

		const speed = await readFile(uci.get('fancontrol', 'settings', 'fan_file'));
		if (speed !== null) {
		    o.description = _('Current speed:') + ` <b>${(speed / 255 * 100).toFixed(2)}%</b>`+'('+speed+')';
	    } else {
		    o.description = _('Error reading fan speed file');
	    }

		// Other options
		o = s.option(form.Value, 'temp_div', _('Temperature coefficient'), _('The temperature coefficient defaults to 1000.'));
		o.placeholder = '1000';

		o = s.option(form.Value, 'start_speed', _('Initial Speed'), _('Please enter the initial speed for fan startup.'));
		o.placeholder = '35';

		o = s.option(form.Value, 'max_speed', _('Max Speed'), _('Please enter maximum fan speed（0-255）.'));
		o.placeholder = '255';

		o = s.option(form.Value, 'start_temp', _('Start Temperature'), _('Please enter the fan start temperature.'));
		o.placeholder = '45';

		return m.render();
	}
});
