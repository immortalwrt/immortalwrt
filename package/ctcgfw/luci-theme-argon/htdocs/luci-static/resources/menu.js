'use strict';
'require baseclass';
'require ui';

return baseclass.extend({
	__init__: function () {
		ui.menu.load().then(L.bind(this.render, this));
	},

	render: function (tree) {
		var node = tree,
			url = '';

		this.renderModeMenu(node);

		if (L.env.dispatchpath.length >= 3) {
			for (var i = 0; i < 3 && node; i++) {
				node = node.children[L.env.dispatchpath[i]];
				url = url + (url ? '/' : '') + L.env.dispatchpath[i];
			}

			if (node)
				this.renderTabMenu(node, url);
		}

		document.querySelector('a.showSide')
			.addEventListener('click', ui.createHandlerFn(this, 'handleSidebarToggle'));
	},

	handleMenuExpand: function (ev) {
		var a = ev.target, slide = a.parentNode, slide_menu = a.nextElementSibling;

		document.querySelectorAll('.main .main-left .nav > li >ul.active').forEach(function (ul) {
			if (ul !== slide) {
				ul.classList.remove('active');
			}

		});

		if (!slide_menu)
			return;

		slide_menu.classList.add('active');
		a.blur();
		ev.preventDefault();
		ev.stopPropagation();
	},

	renderMainMenu: function (tree, url, level) {
		var l = (level || 0) + 1,
			ul = E('ul', { 'class': level ? 'slide-menu' : 'nav' }),
			children = ui.menu.getChildren(tree);

		if (children.length == 0 || l > 2)
			return E([]);

		for (var i = 0; i < children.length; i++) {
			var isActive = ((L.env.dispatchpath[l] == children[i].name) && (L.env.dispatchpath[l - 1] == tree.name)),
				submenu = this.renderMainMenu(children[i], url + '/' + children[i].name, l),
				hasChildren = submenu.children.length,
				activeClass = hasChildren ? 'slide' : null;
			if (isActive) {
				ul.classList.add('active');
				activeClass += " active";
			}

			ul.appendChild(E('li', { 'class': activeClass }, [
				E('a', {
					'href': L.url(url, children[i].name),
					'click': (l == 1) ? ui.createHandlerFn(this, 'handleMenuExpand') : null,
					'class': hasChildren ? 'menu' : null,
					'data-title': hasChildren ? children[i].title.replace(" ", "_") : children[i].title.replace(" ", "_"),
				}, [_(children[i].title)]),
				submenu
			]));
		}

		if (l == 1) {
			document.querySelector('#mainmenu').appendChild(ul);
			document.querySelector('#mainmenu').style.display = '';

		}
		return ul;
	},

	renderModeMenu: function (tree) {
		var menu = document.querySelector('#modemenu'),
			children = ui.menu.getChildren(tree);

		for (var i = 0; i < children.length; i++) {
			var isActive = (L.env.requestpath.length ? children[i].name == L.env.requestpath[0] : i == 0);

			if (i > 0)
				menu.appendChild(E([], ['\u00a0|\u00a0']));

			menu.appendChild(E('li', {}, [
				E('a', {
					'href': L.url(children[i].name),
					'class': isActive ? 'active' : null
				}, [_(children[i].title)])
			]));

			if (isActive)
				this.renderMainMenu(children[i], children[i].name);
		}

		if (menu.children.length > 1)
			menu.style.display = '';
	},

	renderTabMenu: function (tree, url, level) {
		var container = document.querySelector('#tabmenu'),
			l = (level || 0) + 1,
			ul = E('ul', { 'class': 'tabs' }),
			children = ui.menu.getChildren(tree),
			activeNode = null;

		if (children.length == 0)
			return E([]);

		for (var i = 0; i < children.length; i++) {
			var isActive = (L.env.dispatchpath[l + 2] == children[i].name),
				activeClass = isActive ? ' active' : '',
				className = 'tabmenu-item-%s %s'.format(children[i].name, activeClass);

			ul.appendChild(E('li', { 'class': className }, [
				E('a', { 'href': L.url(url, children[i].name) }, [_(children[i].title)])
			]));

			if (isActive)
				activeNode = children[i];
		}

		container.appendChild(ul);
		container.style.display = '';

		if (activeNode)
			container.appendChild(this.renderTabMenu(activeNode, url + '/' + activeNode.name, l));

		return ul;
	},

	handleSidebarToggle: function (ev) {
		var btn = ev.currentTarget,
			bar = document.querySelector('#mainmenu');

		if (btn.classList.contains('active')) {
			btn.classList.remove('active');
			bar.classList.remove('active');
		}
		else {
			btn.classList.add('active');
			bar.classList.add('active');
		}
	}
});
