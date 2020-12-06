/**
 *  Edge is a clean HTML5 theme for LuCI. It is based on luci-theme-Argon
 *
 *  luci-theme-edge
 *      Copyright 2020 Garypang <garyp@qq.com>
 *
 *  Have a bug? Please create an issue here on GitHub!
 *      https://github.com/garypang13/luci-theme-edge/issues
 *
 *  luci-theme-material: 
 *      Copyright 2015 Lutty Yang <lutty@wcan.in>
 *		https://github.com/LuttyYang/luci-theme-material/
 *
 *  Agron Theme
 *	    https://demos.creative-tim.com/argon-dashboard/index.html
 *
 *  Login background
 *      https://unsplash.com/
 *  Font generate by Icomoon<icomoon.io>
 *
 *  Licensed to the public under the Apache License 2.0
 */

document.addEventListener('luci-loaded', function(ev) {
(function ($) {
	$(".main > .loading").fadeOut();

	/**
	 * trim text, Remove spaces, wrap
	 * @param text
	 * @returns {string}
	 */
	function trimText(text) {
		return text.replace(/[ \t\n\r]+/g, " ");
	}

	var lastNode = undefined;
	var mainNodeName = undefined;

	var nodeUrl = "";
	(function(node){
		var luciLocation;
		if (node[0] == "admin"){
			luciLocation = [node[1], node[2]];
		}else{
			luciLocation = node;
		}

		for(var i in luciLocation){
			nodeUrl += luciLocation[i];
			if (i != luciLocation.length - 1){
				nodeUrl += "/";
			}
		}
	})(luciLocation);

	/**
	 * get the current node by Burl (primary)
	 * @returns {boolean} success?
	 */
	function getCurrentNodeByUrl() {
		var ret = false;
		if (!$('body').hasClass('logged-in')) {
			luciLocation = ["Main", "Login"];
			return true;
		}

		$(".main > .main-left > .nav > .slide > .menu").each(function () {
			var ulNode = $(this);
			ulNode.next().find("a").each(function () {
				var that = $(this);
				var href = that.attr("href");

				if (href.indexOf(nodeUrl) != -1) {
					ulNode.click();
					ulNode.next(".slide-menu").stop(true, true);
					lastNode = that.parent();
					lastNode.addClass("active");
					ret = true;
					return true;
				}
			});
		});
		return ret;
	}

	/**
	 * menu click
	 */
	/**
	 * menu click
	 */
	$(".main > .main-left > .nav > .slide > .menu").click(function () {
		var ul = $(this).next(".slide-menu");
		var menu = $(this);
		$(".main > .main-left > .nav > .slide > .menu").each(function () {
			var ulNode = $(this);
			ulNode.removeClass("active");
			ulNode.next(".slide-menu").stop(true).slideUp("fast")
		});
		if (!ul.is(":visible")) {
			menu.addClass("active");
			ul.addClass("active");
			ul.stop(true).slideDown("fast");
		} else {
			ul.stop(true).slideUp("fast", function () {
				menu.removeClass("active");
				ul.removeClass("active");
			});
		}
		return false;
	});

	/**
	 * hook menu click and add the hash
	 */
	$(".main > .main-left > .nav > .slide > .slide-menu > li > a").click(function () {
		if (lastNode != undefined) lastNode.removeClass("active");
		$(this).parent().addClass("active");
		$(".main > .loading").fadeIn("fast");
		return true;
	});

	/**
	 * fix menu click
	 */
	$(".main > .main-left > .nav > .slide > .slide-menu > li").click(function () {
		if (lastNode != undefined) lastNode.removeClass("active");
		$(this).addClass("active");
		$(".main > .loading").fadeIn("fast");
		window.location = $($(this).find("a")[0]).attr("href");
		return false;
	});

	/**
	 * get current node and open it
	 */
	if (getCurrentNodeByUrl()) {
		mainNodeName = "node-" + luciLocation[0] + "-" + luciLocation[1];
		mainNodeName = mainNodeName.replace(/[ \t\n\r\/]+/g, "_").toLowerCase();
		$("body").addClass(mainNodeName);
	}

	/**
	 * Sidebar expand
	 */
	var showSide = false;
	$(".showSide").click(function () {
		if (showSide) {
			$(".darkMask").stop(true).fadeOut("fast");
			$(".main-left").stop(true).animate({
				width: "0"
			}, "200");
			$(".main-right").css("overflow-y", "visible");
			showSide = false;
		} else {
			$(".darkMask").stop(true).fadeIn("fast");
			$(".main-left").stop(true).animate({
				width: "13rem"
			}, "200");
			$(".main-right").css("overflow-y", "hidden");
			showSide = true;
		}
	});

	$(".darkMask").click(function () {
		if (showSide) {
			showSide = false;
			$(".darkMask").stop(true).fadeOut("fast");
			$(".main-left").stop(true).animate({
				width: "0"
			}, "fast");
			$(".main-right").css("overflow-y", "visible");
		}
	});

	$(window).resize(function () {
		if ($(window).width() > 921) {
			$(".main-left").css("width", "");
			$(".darkMask").stop(true);
			$(".darkMask").css("display", "none");
			showSide = false;
		}
	});

	/**
	 * fix legend position
	 */
	$("legend").each(function () {
		var that = $(this);
		that.after("<span class='panel-title'>" + that.text() + "</span>");
	});

	$(".cbi-section-table-titles, .cbi-section-table-descr, .cbi-section-descr").each(function () {
		var that = $(this);
		if (that.text().trim() == ""){
			that.css("display", "none");
		}
	});

	$(".main-right").focus();
	$(".main-right").blur();
	$("input").attr("size", "0");
	$(".cbi-button-up").val("__");
	$(".cbi-button-down").val("__");
	$(".slide > a").removeAttr("href");

	if (mainNodeName != undefined) {
		console.log(mainNodeName);
		switch (mainNodeName) {
			case "node-status-system_log":
			case "node-status-kernel_log":
				$("#syslog").focus(function () {
					$("#syslog").blur();
					$(".main-right").focus();
					$(".main-right").blur();
				});
				break;
			case "node-status-firewall":
				var button = $(".node-status-firewall > .main fieldset li > a");
				button.addClass("cbi-button cbi-button-reset a-to-btn");
				break;
			case "node-system-reboot":
				var button = $(".node-system-reboot > .main > .main-right p > a");
				button.addClass("cbi-button cbi-input-reset a-to-btn");
				break;
		}
	}
	
   var getaudio = $('#player')[0];
   /* Get the audio from the player (using the player's ID), the [0] is necessary */
   var audiostatus = 'off';
   /* Global variable for the audio's status (off or on). It's a bit crude but it works for determining the status. */


   $(document).on('click touchend', '.speaker', function() {
     /* Touchend is necessary for mobile devices, click alone won't work */
     if (!$('.speaker').hasClass("speakerplay")) {
       if (audiostatus == 'off') {
         $('.speaker').addClass('speakerplay');
         getaudio.load();
         getaudio.play();
         audiostatus = 'on';
         return false;
       } else if (audiostatus == 'on') {
         $('.speaker').addClass('speakerplay');
         getaudio.play()
       }
     } else if ($('.speaker').hasClass("speakerplay")) {
       getaudio.pause();
       $('.speaker').removeClass('speakerplay');
       audiostatus = 'on';
     }
   });

   $('#player').on('ended', function() {
     $('.speaker').removeClass('speakerplay');
     /*When the audio has finished playing, remove the class speakerplay*/
     audiostatus = 'off';
     /*Set the status back to off*/
   });
	setTimeout(function(){
var config = {
    // How long Waves effect duration 
    // when it's clicked (in milliseconds)
    duration: 600
};
    Waves.attach("button,input[type='button'],input[type='reset'],input[type='submit']", ['waves-light']);
	// Ripple on hover
$("button,input[type='button'],input[type='reset'],input[type='submit']").mouseenter(function() {
    Waves.ripple(this, {wait: null});
}).mouseleave(function() {
    Waves.calm(this);
});
  Waves.init(config);
$(".waves-input-wrapper").filter(function () {
  if($(this).children().css("display")=="none"){
        return true;
    }else{
        return false;
    }
}).hide();

$("div>select:first-child,div>input[type='text']:first-child").filter(function () {
return (!$(this).parents(".cbi-dynlist").length&&!$("body.Diagnostics").length&&!$(this).hasClass("cbi-input-password"))
}).after("<span class='focus-input'></span>");

$("input[type='checkbox']").filter(function () {
  return (!$(this).next("label").length)
}).show();

$("select,input").filter(function () {
  return ($(this).next(".focus-input").length)
}).focus(function(){
  $(this).css("border-bottom","1px solid #fff");
}).blur(function(){
  $(this).css("border-bottom","1px solid #9e9e9e");
});
	}, 0);

var options = { attributes: true};
function callback() {
$("div>select:first-child,div>input[type='text']:first-child").filter(function () {
return (!$(this).parents(".cbi-dynlist").length&&!$(this).hasClass("cbi-input-password"))
}).after("<span class='focus-input'></span>");
$("select,input").filter(function () {
  return ($(this).next(".focus-input").length)
}).focus(function(){
  $(this).css("border-bottom","1px solid #fff");
}).blur(function(){
  $(this).css("border-bottom","1px solid #9e9e9e");
});
$("input[type='checkbox']").filter(function () {
  return (!$(this).next("label").length)
}).show();
}
var mutationObserver = new MutationObserver(callback);
 mutationObserver.observe($("body")[0], options);
 $(".cbi-value").has("textarea").css("background","none");
})(jQuery);
});