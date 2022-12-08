(function ($, global, undef) {

    var win = $(global);
    var registered = false;

    function hideTip(context) {
        context.visible = false;
        context.tipWrapper.hide();
    }

    function viewportOffset(elm) {
        var offset = $(elm).offset();

        return {
            left: offset.left - win.scrollLeft(),
            top: offset.top - win.scrollTop()
        };
    }

    function scrollTo(context, offset) {
        $('html, body').animate({ scrollTop: offset }, 300);
    }

    function renderTipAtCurrentIndex(context) {
        hideTip(context);

        var tip = context.tips[context.index];
        if (!tip)
            return false;

        context.visible = true;

        var target = $(tip.target);
        // if target is currently hidden, move on to the next tip
        if (!target.is(':visible')) {
            return false;
        }

        // it's effectively fixed if it's fixed or has at least one parent that is
        var isEffectivelyFixed = (
            (target.css('position') == 'fixed') ||
            (target.parents().filter(function () { return $(this).css("position") === 'fixed'; }).length > 0)
        );

        var targetPosition = isEffectivelyFixed ? viewportOffset(target) : target.offset();

        var targetWidth = target.width(),
            targetHeight = target.height(),
            windowWidth = $(window).width(),
            windowHeight = $(window).height();

        // scroll to target's position on page if it isn't fixed
        if (!isEffectivelyFixed) {
            scrollTo(context, targetPosition.top - 150);
        }

        // render tip's content
        context.tipWrapper.html(context.template($.extend({
            message: 'Message Content',
            index: 0,
            count: 1
        }, tip)));

        var tipHeightDifference = context.tipWrapper.children().outerHeight(true);

        // determine where to position
        var alignAbove = false,
            alignRight = false;
        if ((targetPosition.top + targetHeight + context.tipWrapper.height() + tipHeightDifference >= (windowHeight - 200)) &&
            (targetPosition.top - context.tipWrapper.height() - tipHeightDifference >= 0))
            alignAbove = true;
        if (targetPosition.left + context.tipWrapper.width() >= windowWidth - 200)
            alignRight = true;

        var tipPosition = {
            left: (alignRight ? ((targetPosition.left + targetWidth) - context.tipWrapper.width()) : targetPosition.left),
            top: (alignAbove ? targetPosition.top - context.tipWrapper.height() - tipHeightDifference : targetPosition.top + targetHeight)
        };

        // build directional arrow class name
        var arrowClass = (alignAbove ? 'above' : 'below') + (alignRight ? 'right' : 'left');
        context.tipWrapper.find('.tourtip')
            .removeClass('belowleft belowright aboveleft aboveright')
            .addClass(arrowClass);

        var horizontalOffset = -9;
        if (alignRight)
            horizontalOffset = 9;

        // position and show
        context.tipWrapper.css({
            position: (isEffectivelyFixed ? 'fixed' : 'absolute'),
            left: tipPosition.left + horizontalOffset,
            top: tipPosition.top
        }).show();

        return true;
    }

    function moveNext(context) {
        // mark current tip as read before moving forward
        var tip = context.tips[context.index];
        if (tip)
            $.telligent.evolution.tourTips.mark(tip.key, true);

        context.index++;
        var rendered = renderTipAtCurrentIndex(context);
        // if couldn't render, move to next if there are more to show
        if (!rendered && context.index < (context.tips.length - 1))
            moveNext(context)
    }

    function movePrevious(context) {
        context.index--;

        // mark previous tip as unread
        var tip = context.tips[context.index];
        if (tip)
            $.telligent.evolution.tourTips.mark(tip.key, false);

        var rendered = renderTipAtCurrentIndex(context);
        // if couldn't render, move to prev if there are more to show
        if (!rendered && context.index > 0)
            movePrevious(context)
    }
    function stopForever(context) {
        quit(context, true);
        $.telligent.evolution.post({
            url: context.neverShow.url,
            data: {},
            success: function (response) {
                $.telligent.evolution.notifications.show(context.neverShow.success);
            },
            error: function (response) {
                $.telligent.evolution.notifications.show(context.neverShow.failed);
            }
        });
    }

    function quit(context, finished) {
        if (finished) {
            // mark current tip as read
            var tip = context.tips[context.index];
            if (tip)
                $.telligent.evolution.tourTips.mark(tip.key, true);
        }
        scrollTo(context, 0);
        context.tips.splice(0, context.index + 1);
        hideTip(context);
    }

    function buildContext(options) {
        var context = {
            template: $.telligent.evolution.template.compile(options.template),
            tips: [],
            index: 0,
            visible: false,
            suppressDuration: options.suppressDuration,
            tipState: {
                index: function () {
                    return context.index;
                },
                count: function () {
                    return context.tips.length;
                }
            }
        };

        context.tipWrapper = $('<div></div>')
            .css({ position: 'absolute', display: 'none', zIndex: 7000 })
            .appendTo('body')
            .on('click', '.tourtip-previous', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                movePrevious(context);
            })
            .on('click', '.tourtip-next', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                moveNext(context);
            })
            .on('click', '.tourtip-okthanks', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                quit(context, false);
            })
            .on('click', '.tourtip-nevershow', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                stopForever(context);
            })
            .on('click', '.tourtip-finish', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                quit(context, true);
            })
            .on('click', '.tourtip-skip', function (e) {
                e.preventDefault();
                $.fn.evolutionTip.hide();
                $.telligent.evolution.tourTips.suppress(context.suppressDuration * 1000);
                quit(context, false);
            });

        return context;
    }

    function showTips(context) {
        if (context.visible)
            return;

        context.index = 0;
        var rendered = renderTipAtCurrentIndex(context);
        if (rendered) {
            // mark current tip as read
            var tip = context.tips[context.index];
            if (tip)
                $.telligent.evolution.tourTips.mark(tip.key, true);
        }
        // if couldn't render, move to next if there are more to show
        if (!rendered && context.index < (context.tips.length - 1))
            moveNext(context)
    };

    // Add tips to display. If tips are already being displayed
    // then it adds to the list of tips already being shown
    function enqueueTipsToDisplay(context, tips) {
        if (!tips || tips.length === 0) {
            return;
        }

        $.each(tips, function (i, tip) {
            if ($(tip.element).is(':visible')) {
                context.tips.push({
                    message: tip.message,
                    key: tip.key,
                    state: context.tipState,
                    target: tip.element
                });
            }
        });
    }

    var api = {
        register: function (options) {
            // only allow 1 registration
            if (registered)
                return;
            registered = true;

            var context = buildContext({
                template: options.tipTemplate,
                suppressDuration: options.suppressDuration
            });

            context.neverShow = {};
            context.neverShow.url = options.neverShowCallback;
            context.neverShow.success = options.neverShowSuccess;
            context.neverShow.failed = options.neverShowFailure;

            $.telligent.evolution.messaging.subscribe('ui.tourtips', function (data) {
                enqueueTipsToDisplay(context, data.tips);
                // only show tips if wide enough
                if ($(window).width() >= options.minWidth)
                    showTips(context);
            });

            $(document).on('customizepage', function () {
                hideTip(context);
            });
        }
    };

    $.telligent = $.telligent || {};
    $.telligent.evolution = $.telligent.evolution || {};
    $.telligent.evolution.widgets = $.telligent.evolution.widgets || {};
    $.telligent.evolution.widgets.tourTips = api;

})(jQuery, window);
