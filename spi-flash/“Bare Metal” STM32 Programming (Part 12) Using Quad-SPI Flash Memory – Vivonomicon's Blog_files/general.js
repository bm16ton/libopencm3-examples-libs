(function ($) {

"use strict";


function adjustStickyFooter() {
	var footerHeight = $('.footer-wrapper').outerHeight();
	$('body').css('margin-bottom', footerHeight + 'px');
	$('#footer').css('height', footerHeight + 'px');
}


function adjustHtmlMinHeight() {
	if($('body').hasClass('admin-bar')) {
		$('html').css('min-height', $(window).height() - $('#wpadminbar').height() + 'px');
	}
}

function adjustAdminBarPositioning() {
	if ($(window).width() <= 600) {
		$('#wpadminbar').css('position','fixed');
	}
}

function searchPopupController() {
	$(".search-popup-opener").click(function(e){
		e.preventDefault();
		$(".search-popup").addClass('shown');
		$('.section-menu-stripe, .section-logo-area, .section-main-content, .section-footer').addClass('mauer-blur-filter');
		setTimeout(function(){
			$(".search-popup #s").focus();
		}, 200); // needs to be greater than the animation duration
	});

	$(".search-popup-closer").click(function(e){
		e.preventDefault();
		$(".search-popup").removeClass('shown');
		$('.section-menu-stripe, .section-logo-area, .section-main-content, .section-footer').removeClass('mauer-blur-filter');
	});

	$(document).keydown(function(e) {
		if (e.keyCode == 27) {
			$(".search-popup").removeClass('shown');
			$('.section-menu-stripe, .section-logo-area, .section-main-content, .section-footer').removeClass('mauer-blur-filter');
		}
	});
}


function adjustSearchPopupOffset() {
	if($('body').hasClass('admin-bar')) {
		$('.search-popup').css('top', $('#wpadminbar').height() + 'px');
	}
}


function commentFormHighlightNextBorder() {

	$('.comment-respond p.comment-form-author input')
		.mouseenter(function() {
			var urlInput = $(this).closest('p.comment-form-author').next('p.comment-form-email').find('input');
			if (!urlInput.hasClass('mouse-in-the-preceding-input')) {urlInput.addClass(('mouse-in-the-preceding-input'));}
		})
		.mouseleave(function() {
			var urlInput = $(this).closest('p.comment-form-author').next('p.comment-form-email').find('input');
			if (urlInput.hasClass('mouse-in-the-preceding-input')) {urlInput.removeClass(('mouse-in-the-preceding-input'));}
		})
		.focus(function() {
			var urlInput = $(this).closest('p.comment-form-author').next('p.comment-form-email').find('input');
			if (!urlInput.hasClass('focus-on-the-preceding-input')) {urlInput.addClass(('focus-on-the-preceding-input'));}
		})
		.focusout(function() {
			var urlInput = $(this).closest('p.comment-form-author').next('p.comment-form-email').find('input');
			if (urlInput.hasClass('focus-on-the-preceding-input')) {urlInput.removeClass(('focus-on-the-preceding-input'));}
		});

	$('.comment-respond p.comment-form-email input')
		.mouseenter(function() {
			var urlInput = $(this).closest('p.comment-form-email').next('p.comment-form-url').find('input');
			if (!urlInput.hasClass('mouse-in-the-preceding-input')) {urlInput.addClass(('mouse-in-the-preceding-input'));}
		})
		.mouseleave(function() {
			var urlInput = $(this).closest('p.comment-form-email').next('p.comment-form-url').find('input');
			if (urlInput.hasClass('mouse-in-the-preceding-input')) {urlInput.removeClass(('mouse-in-the-preceding-input'));}
		})
		.focus(function() {
			var urlInput = $(this).closest('p.comment-form-email').next('p.comment-form-url').find('input');
			if (!urlInput.hasClass('focus-on-the-preceding-input')) {urlInput.addClass(('focus-on-the-preceding-input'));}
		})
		.focusout(function() {
			var urlInput = $(this).closest('p.comment-form-email').next('p.comment-form-url').find('input');
			if (urlInput.hasClass('focus-on-the-preceding-input')) {urlInput.removeClass(('focus-on-the-preceding-input'));}
		});

}


function mauerInstafeed() {
	if ($('#mauer-instafeed-settings').length) {
		var nuOfPics = 10;

		var minHeightAtLoad = $(window).width() / nuOfPics;
		var relativeWidth = 100 / nuOfPics;
		$('#mauer-instafeed').css('min-height', minHeightAtLoad + 'px');

		var token  = $('#mauer-instafeed-settings #accessToken').text();

		var feed = new Instafeed({
			get: 'user',
			userId: token.substr(0, token.indexOf('.')),
			accessToken: token,
			resolution: 'low_resolution',
			limit: nuOfPics,
			target: 'mauer-instafeed',
			template: '<a href="{{link}}" class="mauer-instafeed-thumb-link" style="width:' + relativeWidth + '%;"><img src="{{image}}" /><div class="instafeed-thumb-overlay"></div></a>',
			error: function(error) {
				var errorIntro = $($('#mauer-instafeed')).data('errorIntro');
				$('#mauer-instafeed').append(errorIntro + '"' + error + '"');
			}
		});
		feed.run();
	}
}

function mauerInstafeedMinHeightRemover() { $('#mauer-instafeed').css('min-height', ''); }


function adjustEmbeddediFrameDimensions() {
	// preserve aspect ratio of all iframes that have width and height attributes set.
	$("iframe").each(function(i){
		if ( $.isNumeric($(this).attr("width")) && $.isNumeric($(this).attr("height")) ) {
			var aspectRatio = $(this).attr("width") / $(this).attr("height");
			$(this).height( $(this).width() / aspectRatio );
		}
	});

	// make iframes in the entry-full area wider.
	$(".entry-full iframe").each(function(i, element) {
		var aspectRatio = $(element).width() / $(element).height();
		var widthToHave = $('.container').width();
		var heightToHave = $('.container').width() / aspectRatio;
		if (!$('.widgetized-area').length) {
			$(element).width(widthToHave).height(heightToHave);
			$(element).css({
				'max-width': 'none',
				'position': 'relative',
				'left': ($('.entry-content').width() - widthToHave)/2,
			});
		}
	});
}


$(document).ready(function() {
	adjustHtmlMinHeight();
	adjustSearchPopupOffset();
	searchPopupController();
	mauerInstafeed();
	adjustStickyFooter();
	commentFormHighlightNextBorder();
	adjustAdminBarPositioning();
});


$(window).resize(function(){
	mauerInstafeedMinHeightRemover();
	adjustHtmlMinHeight();
	adjustEmbeddediFrameDimensions();
	adjustStickyFooter();
	adjustSearchPopupOffset();
	adjustAdminBarPositioning();
});

$(window).load(function(){
	$(".mauer-preloader").addClass("mauer-preloader-hidden");
	adjustEmbeddediFrameDimensions();
});


})(jQuery);
