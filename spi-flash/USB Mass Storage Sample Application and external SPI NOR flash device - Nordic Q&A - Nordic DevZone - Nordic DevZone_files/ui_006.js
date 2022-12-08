(function ($, global, undef) {

    if (!$.ns) { $.ns = {}; }
    if (!$.ns.widgets) { $.ns.widgets = {}; }

    function throttle(fn, limit) {
        var lastRanAt, timeout;
        return function () {
            var scope = this
            attemptAt = (new Date().getTime()),
                args = arguments;
            if (lastRanAt && (lastRanAt + (limit || 50)) > attemptAt) {
                clearTimeout(timeout);
                timeout = setTimeout(function () {
                    lastRanAt = attemptAt;
                    fn.apply(scope, args);
                }, (limit || 50));
            } else {
                lastRanAt = attemptAt;
                fn.apply(scope, args);
            }
        };
    }

    $.ns.widgets.nordicSiteBannerCustomisation = {
        register: function (context) {

            // :focus-within polyfill
            if (typeof focusWithin === 'function') {
                focusWithin(document);
            }

            // get handles to header regions
            var $siteHeader = $('.header-fragments:visible');
            var $siteBanner = $siteHeader.find('.site-banner');
            var $siteNavigation = $siteHeader.find('.nordic-site-banner-customisation');
            var $siteNavigationItems = $siteNavigation.find('.navigation-list .navigation-list-item');
            var $fieldset = $siteBanner.find('fieldset.search').first();
            var $searchWrapper = $fieldset.find('.field-list');

            // inject internal fieldset wrapper (as fieldset does not support CSS flexbox / grid)
            window.setTimeout(function () {
                $fieldset.children().wrapAll("<div class='fieldset-wrapper' />");
                $fieldset.addClass("updated");
            }, 500);

            // inject support link after search field
            var $support = $('<li class="field-item" />').append(
                $('<a href="' + context.addTicketUrl + '" class="add-ticket-button" />').append(
                    $('<span>' + context.supportLabel + ' </span><span class="plus">+<span />')
                )
            );

            $searchWrapper.append($support);

            // update search placeholder text
            $searchWrapper.find("input").first().attr("placeholder", context.searchPlaceholder);

            // insert links above search field / support button
            var $navLinksWrapper = $('<div class="nav-links-wrapper" />').append(
                    $('<button type="button" class="nav-links-burger" />')
            );
            var $navLinks = $("<ul class='nav-links' />");

            $.each($siteNavigationItems, function (i, link) {
                $navLinks.append(link);
            });
            $navLinksWrapper.append($navLinks);

            $fieldset.prepend($navLinksWrapper);

            // inject DevZone logo home link
            var $devZone = $('<a href="/" class="devzone-logo-link" />').append(
                $('<img alt="' + context.homeLabel + '" src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALUAAABKCAMAAADKSeVXAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4QgCCwYs42LFSQAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAAAXVBMVEUAAADU3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i/////U3i////+6dG8oAAAAHXRSTlMAEREiIjMzRERVVWZmd3eIiJmZqqq7u8zM3d3u7lD7mhcAAAABYktHRACIBR1IAAADLElEQVRo3u2bXZeiMAyGY6fTVdQu6yAiU97//zP3otjSLxaU407Paa8YZZLHlxCSWIm8dTh/D3qdKZd1uA9mZUP9NQz5UV+HDKkdpXOhPmvY73zuQiLa6dxx3eUEPUp9zwuabsMwDMMxL2jSUZ2Z1B86QDKT+lehLtSFulAX6kL9P6hbjKs9PWWaw1nyzdRAx3OkRs8zor4QEbELgPYlLycAp/dSE0kA+1e89EBP76amDqh/kNQLqWug+0FSL6SWjlvZA7jsHSmZ+esSIAZSM98CMSklJ1EDaLxYDM41ze7XCuq9GpNCY1w6VCqIptb7HKEF4gD2MpJtIueaZve8nLqyuczklW5iVAAQjp29B1IBqAXJfmKBA2iM4co51/dGdNTN7sdiaq4AdSLizYRFAooix8aOYpM0rsaExDprQed2yXS+6qfnBt7oU0NfaTF1DShujpjVd59KN77UtTHLrQUOQAmTZgXNeBuVHj6XU1t9mLKX0qIy5wJHpKbenmAPuf03puynDL2dz/f5eWSU2lVVBpmx8hgDqacWevMWj9v1Xz3aIeofeo66tdaFyX0X724PpOb/pG6j1K1LnRyWraAmc1WVl5o5vDjfgvp2Ts/KUtSxOu4hsXAeOPoN8FeoHW8Pre+HTairMd9J79HPYY1sSD0zuV5DzcYk3Hm3XiD1VtTDbfdcXDurAWqd98Ss1E/Hta1Bxq8YbxtQ69CovAAJpX6dmnbXuTSyipoBYKPiDmFDW1PT7jYzWFhFrSNaue/GeqANqMeh+3BY3BUkqSXQCLdyYirWb25APX7Z+HtxBzZJEifncSIAJd17L95uKge1mqVOeTum78dot2urMKHQMjdElFM5xaWm2lR3jVvzRahT3pZMFupJTf6oeHUdLFwar5COX9+HBdHCra8j1Clvq6c4k+5CCb+4czIGTz9Hg/4kQZ3ytn5iVj06uU4EAes8GJOzpyrRN0aoE96emE7qDrsJBhwXr3JKT8zCHj1JHfdW5teFulAX6kJdqAt1oS7Ua1aeu7Xy3BmX6S7EPHd85rm7Ns+dzJnuGs90h36mv4bI9JcnlMmvfP4CCdXOn3NvKrAAAAAASUVORK5CYII=" />')
            );

            $fieldset.prepend($devZone);

            // auto-resize and position search results as necessary
            var $searchInput = $fieldset.find("input[type='search']");

            updateSearchPosition();

            function updateSearchPosition() {
                var $searchResults = $(".popup-list.search");

                if ($searchResults.length) {
                    $searchResults[0].style.removeProperty('margin-top');
                    $searchResults[0].style.removeProperty('margin-left');
                    $searchResults[0].style.removeProperty('min-width');
                    $searchResults[0].style.removeProperty('max-width');

                    // indent the search results dropdown and match input field width
                    var left = $fieldset.offset().left + 150 + 'px';
                    var width = $searchInput.outerWidth() + 'px';

                    // layout swaps @ 1200px
                    if (window.matchMedia('(max-width: 1200px)').matches) {
                        left = $fieldset.offset().left + 60 + 'px';
                    }

                    // padding reduces < 1024px
                    if (window.matchMedia('(max-width: 1023px)').matches) {
                        left = $fieldset.offset().left + 50 + 'px';
                    }

                    // full width at mobile res
                    if (!window.matchMedia('(max-width: 670px)').matches) {
                        // NB: not using left / width properties as these are regularly updated by Telligent; avoids flicker
                        $searchResults[0].style.setProperty('margin-left', left, 'important');
                        $searchResults[0].style.setProperty('min-width', width, 'important');
                        $searchResults[0].style.setProperty('max-width', width, 'important');
                    }

                }
            }

            // make sure the search results position is updated on the very first and all the subsequent activations
            var searchActivateMessage = 'widget.' + $('.content-fragment.site-banner').attr('id') + '.telligent.evolution.widgets.siteBanner.activate';
            $.telligent.evolution.messaging.subscribe(searchActivateMessage, function (data) {
                setTimeout(updateSearchPosition, 5);
            });

            $(window).on("resize", throttle(updateSearchPosition, 100));
            $searchInput.on("keydown", updateSearchPosition);
            $(window).on("orientationchange", updateSearchPosition);

            var $navLinksBurger = $(".nav-links-burger");

            // click handler for burger menu
            $navLinksBurger.on("mousedown", function (e) {
                e.preventDefault();

                if (document.activeElement === e.target) {
                    e.target.blur();
                } else if ($navLinksBurger[0].parentElement.contains(document.activeElement)) {
                    document.activeElement.blur();
                } else {
                    // necessary for Safari / Firefox on Mac
                    e.target.focus();
                }

            });

            // ensure focus-within behaviour behaves on touch devices
            $navLinksBurger.on("touchend", function (e) {
                e.preventDefault();
                if (document.activeElement !== e.target) {
                    e.target.focus();
                } else {
                    e.target.blur();
                }
            });

            var mut = new MutationObserver(function (mutations, mut) {
                // if attribute changed === 'class' && 'open' has been added, add css to 'otherDiv'
                if (!context.menuAppended) {
                    var source = context.selectors.siteBannerMergeTemplate;

                    var markupSource = $('<span></span>');

                    markupSource.append(source.html());

                    var popup = $('.popup-list.user .navigation-list.user');

                    if (popup.length > 0) {
                        popup.prepend(markupSource.html());

                        context.menuAppended = true;
                    }

                    $('details.private-ticket-groups').on('mouseover click', function() {
                        $(this).attr('open', true);
                    }).on('mouseout', function() {
                        $(this).attr('open', false);
                    })

                }
            });

            mut.observe($('.site-banner .navigation-list.user-links .internal-link.user')[0], {
                'attributes': true
            });
        }
    };

})(jQuery, window);