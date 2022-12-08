
(function(){
		(function ($, global, undef) {

	if (!$.telligent) {
		$.telligent = {};
	}
	if (!$.telligent.evolution) {
		$.telligent.evolution = {};
	}
	if (!$.telligent.evolution.widgets) {
		$.telligent.evolution.widgets = {};
	}

	var messaging = $.telligent.evolution.messaging;

	var model = {
		unlinkWiki: function (context, threadId) {
			return $.telligent.evolution.post({
				url: context.unlinkUrl,
				data: {
					threadId: threadId
				}
			});
		},
		muteThread: function (context, threadId) {
			return $.telligent.evolution.post({
				url: context.muteUrl,
				data: {
					type: 'forumThread',
					mute: true,
					forumThreadId: threadId
				}
			});
		},
		unMuteThread: function (context, threadId) {
			return $.telligent.evolution.post({
				url: context.muteUrl,
				data: {
					type: 'forumThread',
					mute: false,
					forumThreadId: threadId
				}
			});
		},
		unSubscribeThread: function (context, threadId) {
			return $.telligent.evolution.post({
				url: context.subscribeUrl,
				data: {
					type: 'forumThread',
					subscribe: false,
					forumThreadId: threadId
				}
			});
		},
		subscribeThread: function (context, threadId) {
			return $.telligent.evolution.post({
				url: context.subscribeUrl,
				data: {
					type: 'forumThread',
					subscribe: true,
					forumThreadId: threadId
				}
			});
		},
		deleteThread: function (context, forumId, threadId) {
			var data = {
				ForumId: forumId,
				ThreadId: threadId,
				DeleteChildren: true,
				SendAuthorDeleteNotification: true,
				DeleteReason: "Deleted via UI"
			};
			return $.telligent.evolution.del({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/{ForumId}/threads/{ThreadId}.json',
				data: data
			});
		},
		voteThread: function (context, threadId) {
			return $.telligent.evolution.post({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/vote.json',
				data: {
					ThreadId: threadId
				}
			});
		},
		unvoteThread: function (context, threadId) {
			return $.telligent.evolution.del({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/vote.json',
				data: {
					ThreadId: threadId
				}
			});
		},
		getThreadVoteCount: function (context, threadId) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/votes.json',
				data: {
					ThreadId: threadId,
					PageSize: 1,
					PageIndex: 0
				}
			}).then(function (d) {
				return d.TotalCount;
			});
		},
		addReply: function (context, forumId, threadId, body, suggestAsAnswer, parentId) {
			var data = {
				ForumId: forumId,
				ThreadId: threadId,
				Body: body,
				SubscribeToThread: true,
				IsSuggestedAnswer: suggestAsAnswer
			};
			if (parentId) {
				data.ParentReplyId = parentId;
			}
			var result = $.telligent.evolution.post({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/{ForumId}/threads/{ThreadId}/replies.json',
				data: data
			});
			result.then(function(r){
				eventSynthesizer.replyCreated(context, r.Reply.Id);
			});
			return result;
		},
		updateReply: function (context, forumId, threadId, replyId, body, suggestAsAnswer) {
			var result = $.telligent.evolution.put({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/{ForumId}/threads/{ThreadId}/replies/{ReplyId}.json',
				data: {
					ForumId: forumId,
					ThreadId: threadId,
					ReplyId: replyId,
					Body: body,
					IsSuggestedAnswer: suggestAsAnswer
				}
			});
			result.then(function(r){
				eventSynthesizer.replyUpdated(context, replyId);
			});
			return result;
		},
		storeRootReplyToTempData: function (context, threadId, body, suggestAsAnswer) {
			return $.telligent.evolution.post({
				url: context.advancedReplyUrl,
				data: {
					threadId: threadId,
					replyBody: $.telligent.evolution.html.decode(body),
					isSuggestion: suggestAsAnswer
				}
			});
		},
		lockThread: function (context, forumId, threadId) {
			return $.telligent.evolution.put({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/{ForumId}/threads/{ThreadId}.json',
				data: {
					ForumId: forumId,
					ThreadId: threadId,
					IsLocked: true
				}
			});
		},
		unlockThread: function (context, forumId, threadId) {
			return $.telligent.evolution.put({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/{ForumId}/threads/{ThreadId}.json',
				data: {
					ForumId: forumId,
					ThreadId: threadId,
					IsLocked: false
				}
			});
		},
		moderateUser: function (context, userId) {
			return $.telligent.evolution.put({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/users/{UserId}.json',
				data: {
					UserId: userId,
					ModerationLevel: 'Moderated'
				}
			});
		},
		unModerateUser: function (context, userId) {
			return $.telligent.evolution.put({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/users/{UserId}.json',
				data: {
					UserId: userId,
					ModerationLevel: 'Unmoderated'
				}
			});
		},
		voteReply: function (context, replyId) {
			// up-vote the quality of the reply along with voting up the answer/verification
			return $.Deferred(function(d) {
				$.telligent.evolution.post({
					url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
					data: {
						ReplyId: replyId,
						VoteType: 'Quality',
						Value: true
					}
				}).then(function(){
					$.telligent.evolution.post({
						url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
						data: {
							ReplyId: replyId
						}
					}).then(function(r) {
						eventSynthesizer.replyUpdated(context, replyId);
						d.resolve(r);
					}, function(r) {
						d.reject(r);
					});
				})
			}).promise();
		},
		unvoteReply: function (context, replyId) {
			var result = $.telligent.evolution.del({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
				data: {
					ReplyId: replyId
				}
			});
			result.then(function(r){
				eventSynthesizer.replyUpdated(context, replyId);
			});
			return result;
		},
		getThread: function (context, threadId) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}.json',
				data: {
					ThreadId: threadId
				}
			});
		},
		getReply: function (context, replyId) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}.json',
				data: {
					ReplyId: replyId
				}
			});
		},
		getReplyVoteCount: function (context, replyId) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/votes.json',
				data: {
					ReplyId: replyId,
					PageSize: 1,
					PageIndex: 0
				}
			}).then(function (d) {
				return d.TotalCount;
			});
		},
		storeReplyToTempData: function (context, options) {
			var data = {
				threadId: options.threadId,
				replyBody: $.telligent.evolution.html.decode(options.body),
				isSuggestion: options.suggestAsAnswer
			};

			if (options.parentReplyId)
				data.parentReplyId = options.parentReplyId;
			if (options.replyId)
				data.replyId = options.replyId;

			return $.telligent.evolution.post({
				url: context.advancedReplyUrl,
				data: data
			});
		},
		suggest: function (context, replyId) {
			// up-vote the quality of the reply along with suggesting it as an answer
			return $.Deferred(function(d) {
				$.telligent.evolution.post({
					url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
					data: {
						ReplyId: replyId,
						VoteType: 'Quality',
						Value: true
					}
				}).then(function(){
					$.telligent.evolution.post({
						url: context.editReplyUrl,
						data: prefix({
							replyId: replyId,
							suggestAnswer: true
						})
					}).then(function(r) {
						eventSynthesizer.replyUpdated(context, replyId);
						d.resolve(r);
					}, function(r) {
						d.reject(r);
					});
				})
			}).promise();
		},
		getBestReplies: function (context) {
			return $.telligent.evolution.get({
				url: context.listBestUrl
			});
		}
	};

	// synthesizes socket-based forum messages, attempting
	// to use locally-known data, otherwise falling back to
	// requesting data from REST to raise with synthesized events
	// events are only raised for accessing user,
	// as the actual socket events for these messages exclude
	// sending to the originating user
	var eventSynthesizer = (function(){
		var raise = function(message, data) {
			$.telligent.evolution.messaging.publish(message, data, {
				crossWindow: true,
				excludeOrigin: true
			});
		};
		var synthesize = function(message, context, replyId, data) {
			if (data) {
				raise(message, $.extend(data, { local: true }));
			} else {
				model.getReply(context, replyId).then(function(r){
					model.getThread(context, r.Reply.ThreadId).then(function(t){
						raise(message, {
							replyId: replyId,
							threadId: r.Reply.ThreadId,
							forumId: r.Reply.ForumId,
							parentId: r.Reply.ParentId,
							authorId: r.Reply.Author.Id,
							approved: r.Reply.Approved,
							url: r.Reply.Url,
							status: t.Thread.ThreadStatus,
							replyCount: t.Thread.ReplyCount,
							answerCount: t.Thread.AnswerCount + t.Thread.SuggestedAnswerCount,
							isAuthor: (r.Reply.Author.Id == $.telligent.evolution.user.accessing.id),
							local: true
						});
					});
				});
			}
		}

		return {
			replyCreated: function(context, replyId, data) {
				synthesize('forumReply.created', context, replyId, data);
			},
			replyUpdated: function(context, replyId, data) {
				synthesize('forumReply.updated', context, replyId, data);
			}
		}
	})();

	function loadMoreStartActions(context, options) {
		return $.telligent.evolution.get({
			url: context.moreStartActionsUrl,
			data: options
		});
	}

	function validate(editor) {
		return $.trim(editor.val()).length > 0;
	}

	function createChildReply(context, body, forumId, threadId, parentId, suggestAnswer) {
		if (context.creating)
			return;
		context.creating = true;

		context.loadingIndicator = context.loadingIndicator || $('#' + context.wrapperId).find('.processing');
		context.loadingIndicator.removeClass('hidden');

		return model.addReply(context,
				forumId,
				threadId,
				body,
				suggestAnswer,
				parentId)
			.then(function (r) {
				context.replyEditor.checkedVal(false);
				context.replyEditor.val('');
				context.creating = false;
				context.loadingIndicator.addClass('hidden');

				if (!r.Reply.Approved) {
					$.telligent.evolution.notifications.show(context.replyModerated, {
						type: 'warning'
					});
				} else {
					$.telligent.evolution.notifications.show(context.successMessage, {
						duration: 3 * 1000
					});
				}
			})
			.catch(function () {
				context.creating = false;
				context.loadingIndicator.addClass('hidden');
			});
	}

	function createRootReply(context, body, forumId, threadId, suggestAnswer, position) {
		if (context.creating)
			return;
		context.creating = true;

		context.loadingIndicator = context.loadingIndicator || $('#' + context.wrapperId).find('.processing');
		context.loadingIndicator.removeClass('hidden');

		var editor = getCreateEditor(context, position);

		return model.addReply(context,
				forumId,
				threadId,
				body,
				suggestAnswer)
			.then(function (r) {

				editor.checkedVal(false);
				editor.val('');
				context.creating = false;
				context.loadingIndicator.addClass('hidden');

				if (!r.Reply.Approved) {
					$.telligent.evolution.notifications.show(context.replyModerated, {
						type: 'warning'
					});
				} else {
					$.telligent.evolution.notifications.show(context.successMessage, {
						duration: 3 * 1000
					});
				}

				$.telligent.evolution.messaging.publish('forumReply.created.root', {
					replyId: r.Reply.Id,
					threadId: r.Reply.ThreadId,
					approved: r.Reply.Approved,
					url: r.Reply.Url,
					authorId: $.telligent.evolution.user.accessing.id,
					isAuthor: true,
					local: true
				});
			})
			.catch(function () {
				context.creating = false;
				context.loadingIndicator.addClass('hidden');
			});
	}

	function prefix(options) {
		var data = {};
		$.each(options, function (k, v) {
			data['_w_' + k] = v;
		});
		return data;
	}

	function throttle(fn, limit) {
		var lastRanAt, timeout;
		return function () {
			var scope = this,
				attemptAt = (new Date().getTime()),
				args = arguments;
			if (lastRanAt && (lastRanAt + (limit || 50)) > attemptAt) {
				global.clearTimeout(timeout);
				timeout = global.setTimeout(function () {
					lastRanAt = attemptAt;
					fn.apply(scope, args);
				}, (limit || 50));
			} else {
				lastRanAt = attemptAt;
				fn.apply(scope, args);
			}
		};
	}

	function handleEvents(context) {

		messaging.subscribe('telligent.evolution.widgets.thread.unlinkwiki', function (data) {
			var link = $(data.target);
			model.unlinkWiki(context, link.data('threadid')).then(function () {
				link.closest('.message').hide();
			});
		});

		messaging.subscribe('telligent.evolution.widgets.thread.mute', function (data) {
			var link = $(data.target);
			if (link.data('mute')) {
				link.html('...');
				model.unMuteThread(context, link.data('threadid')).then(function () {
					link.html(link.data('offtext')).data('mute', false);
					$('#' + link.data('links')).uilinks('hide');
				});
			} else {
				link.html('...');
				model.muteThread(context, link.data('threadid')).then(function () {
					link.html(link.data('ontext')).data('mute', true);
					$('#' + link.data('links')).uilinks('hide');
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.subscribe', function (data) {
			var link = $(data.target);
			if (link.data('subscribed')) {
				link.html('...');
				model.unSubscribeThread(context, link.data('threadid')).then(function () {
					link.html(link.data('offtext')).data('subscribed', false);
					$('#' + link.data('links')).uilinks('hide');
				});
			} else {
				link.html('...');
				model.subscribeThread(context, link.data('threadid')).then(function () {
					link.html(link.data('ontext')).data('subscribed', true);
					$('#' + link.data('links')).uilinks('hide');
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.deletethread', function (data) {
			var link = $(data.target);
			if (confirm(context.confirmDeleteThreadMessage)) {
				model.deleteThread(context,
						link.data('forumid'),
						link.data('threadid'))
					.then(function (data) {
						global.location = link.data('forumurl');
					});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.composereply', function (data) {
			var link = $(data.target);
			link.addClass('hidden');
			$('#' + link.data('cancelid')).removeClass('hidden');
			$('#' + link.data('replyformid')).removeClass('hidden').find('textarea').trigger('focus').trigger('focus');
			context.createEditor.focus();
		});

		messaging.subscribe('telligent.evolution.widgets.thread.cancelreply', function (data) {
			var link = $(data.target);
			link.addClass('hidden');
			$('#' + link.data('composeid')).removeClass('hidden');
			$('#' + link.data('replyformid')).addClass('hidden');
		});

		messaging.subscribe('telligent.evolution.widgets.thread.capture', function (data) {
			var link = $(data.target);
			Telligent_Modal.Open(link.data('captureurl'), 550, 300, null);
		});

		messaging.subscribe('telligent.evolution.widgets.thread.submit', function (data) {
			if (data.from != context.wrapperId + '-root')
				return;

			var editor = getCreateEditor(context, data.position);

			if (!validate(editor))
				return;

			var body = $.trim(editor.val());
			var suggestAnswer = editor.checkedVal();

			createRootReply(context, body, data.forumId, data.threadId, suggestAnswer, data.position);
		});

		messaging.subscribe('telligent.evolution.widgets.thread.fullEditor.start', function (data) {
			var link = $(data.target);
			var suggestAnswer = context.createEditor.checkedVal();
			var body = context.createEditor.val();

			model.storeRootReplyToTempData(context,
					link.data('threadid'),
					body,
					suggestAnswer)
				.then(function (r) {
					global.location = r.replyUrl;
				});
		});

		messaging.subscribe(context.moreStartLinkMessage, function (e) {
			var moreLink = $(e.target);
			var links = moreLink.closest('.navigation-list');

			if (links.data('extra_links_loaded')) {
				links.uilinks('show', $(e.target));
			} else {
				links.data('extra_links_loaded', true);
				loadMoreStartActions(context, {
					forumApplicationId: moreLink.data('forumapplicationid'),
					threadContentId: moreLink.data('threadcontentid'),
					replyContentId: moreLink.data('replycontentid'),
					onAReplyPage: moreLink.data('onareplypage'),
					replyCount: moreLink.data('replycount'),
					replyPageIndex: moreLink.data('replypageindex'),
					postActionsId: moreLink.data('postactionsid')
				}).then(function (response) {
					$(response).children('li.navigation-list-item').each(function () {
						var cssClass = $(this).attr('class');
						var link = $(this).children('a, span').first();
						links.uilinks('add', link);
					});
					links.uilinks('show', moreLink);
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.typing', throttle(function (data) {
			if (data.from != context.wrapperId)
				return;

			sendTyping(context, {
				threadId: context.threadId
			});
		}, 1500));

		messaging.subscribe('telligent.evolution.widgets.thread.login', function (data) {
			var loginUrl = $.telligent.evolution.url.modify({
				url: context.loginUrl,
				query: {
					ReturnUrl: $.telligent.evolution.url.modify({
						url: $(data.target).data('replyurl') || global.location.href,
						query: {
							focus: 'true'
						}
					})
				}
			});
			global.location.href = loginUrl;
		});

		messaging.subscribe('ui-forumvote', function (data) {
			if (data.id == context.threadId && data.type == 'reply') {
				$(elm).html(data.count).attr('data-count', data.count);
				if (data.voted) {
					$(elm).attr('data-voted', 'true');
				} else {
					$(elm).attr('data-voted', 'false');
				}
			}
		});

		messaging.subscribe('ui-forumvote', function (data) {
			if (data.id != context.threadId)
				return;
			if (data.voted) {
				$(context.container).addClass('has-question');
			} else {
				$(context.container).removeClass('has-question');
			}
		});

		messaging.subscribe(context.moreLinkMessage, function (e) {
			var moreLink = $(e.target);
			var links = moreLink.closest('.navigation-list');

			if (links.data('extra_links_loaded')) {
				links.uilinks('show', $(e.target));
			} else {
				links.data('extra_links_loaded', true);
				loadMoreReplyActions(context, {
					forumApplicationId: moreLink.data('forumapplicationid'),
					threadContentId: moreLink.data('threadcontentid'),
					replyContentId: moreLink.data('replycontentid'),
					forumReplyActionsId: moreLink.data('forumreplyactionsid')
				}).then(function (response) {
					$(response).children('li.navigation-list-item').each(function () {
						var cssClass = $(this).attr('class');
						var link = $(this).children('a, span').first();
						links.uilinks('add', link);
					});
					links.uilinks('show', moreLink);
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.lock', function (data) {
			var link = $(data.target);
			if (link.data('locked')) {
				link.html('...');
				model.unlockThread(context, link.data('forumid'), link.data('threadid')).then(function () {
					link.html(link.data('offtext')).data('locked', false);
					$('#' + link.data('links')).uilinks('hide');
				});
			} else {
				link.html('...');
				model.lockThread(context, link.data('forumid'), link.data('threadid')).then(function () {
					link.html(link.data('ontext')).data('locked', true);
					$('#' + link.data('links')).uilinks('hide');
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.moderateuser', function (data) {
			var link = $(data.target);
			if (link.data('moderated')) {
				link.html('...');
				model.unModerateUser(context, link.data('userid')).then(function () {
					link.html(link.data('offtext')).data('moderated', false);
					$('#' + link.data('links')).uilinks('hide');
				});
			} else {
				link.html('...');
				model.moderateUser(context, link.data('userid')).then(function () {
					link.html(link.data('ontext')).data('moderated', true);
					$('#' + link.data('links')).uilinks('hide');
				});
			}
		});

		messaging.subscribe('telligent.evolution.widgets.thread.viewattachment', function (data) {
			var link = $(data.target);
			link.hide();
			link.closest('.attachment').find('.hide-attachment a').show().removeClass('hidden');
			link.closest('.attachment').find('.viewer').show().removeClass('hidden');
		});

		messaging.subscribe('telligent.evolution.widgets.thread.hideattachment', function (data) {
			var link = $(data.target);
			link.hide();
			link.closest('.attachment').find('.view-attachment a').show().removeClass('hidden');
			link.closest('.attachment').find('.viewer').hide().addClass('hidden');
		});

		messaging.subscribe('telligent.evolution.widgets.thread.votereply', function (data) {
			var link = $(data.target);
			model.voteReply(context, link.data('replyid')).then(function (rep) {
				link.hide();
				$('#' + link.data('unvotelink')).show();
				model.getReplyVoteCount(context, link.data('replyid')).then(function (data) {
					messaging.publish('ui-forumvote', {
						type: 'reply',
						id: link.data('replyid'),
						count: data,
						voted: true
					});
				});
			});
		});

		messaging.subscribe('telligent.evolution.widgets.thread.unvotereply', function (data) {
			var link = $(data.target);
			model.unvoteReply(context, link.data('replyid')).then(function (rep) {
				link.hide();
				$('#' + link.data('votelink')).show();
				model.getReplyVoteCount(context, link.data('replyid')).then(function (data) {
					messaging.publish('ui-forumvote', {
						type: 'reply',
						id: link.data('replyid'),
						count: data,
						voted: false
					});
				});
			});
		});

		messaging.subscribe('telligent.evolution.widgets.thread.fullEditor', function (data) {
			var replyForm = context.currentEditorParentContainer.closest('.reply-form');
			var replyOrParentId = replyForm.closest('.content-item').data('id');
			var isNew = replyForm.closest('.newreply').length > 0;

			//threadId, parentReplyId, body, suggestAsAnswer
			model.storeReplyToTempData(context, {
				threadId: context.threadId,
				parentReplyId: (isNew ? replyOrParentId : null),
				replyId: (isNew ? null : replyOrParentId),
				body: context.replyEditor.val(),
				suggestAsAnswer: context.replyEditor.checkedVal()
			}).then(function (r) {
				global.location = r.replyUrl;
			});
		});

		messaging.subscribe('telligent.evolution.widgets.thread.suggest', function (data) {
			var link = $(data.target);
			model.suggest(context, link.data('replyid')).then(function (rep) {
				link.hide();
			});
		});

		$(context.wrapper).on('click', '.content-item.thumbnail', function (e) {
			window.location = $(this).data('url');
		});

		$(context.wrapper).on('quote', '.content.full .content', function (e) {
			var c = $(e.target).closest('.content.full');
			var authorId = c.data('userid');
			var replyId = c.data('replyid');
			var threadId = c.data('threadid');
			var url = c.data('permalink');

			if (context.currentEditorParentContainer != null) {
				// has an open reply editor
				context.replyEditor.insert('[quote userid="' + (authorId || '') + '" url="' + (url || '') + '"]' + e.quotedHtml + '[/quote]');
				context.replyEditor.focus();
				return;
			}

			if (!$('#' + context.createWrapperId).hasClass('hidden')) {
				getCreateEditor(context, 'header').insert('[quote userid="' + (authorId || '') + '" url="' + (url || '') + '"]' + e.quotedHtml + '[/quote]');
				getCreateEditor(context, 'header').focus();
				return;
			}

			if (replyId) {
				var reply = $('.content-item[data-id="' + replyId + '"]');
				if (reply.length > 0) {
					reply.find('a.new-reply').first().trigger('click');
					global.setTimeout(function () {
						context.replyEditor.insert('[quote userid="' + (authorId || '') + '" url="' + (url || '') + '"]' + e.quotedHtml + '[/quote]');
						context.replyEditor.focus();
						return;
					}, 250);
					return;
				}
			}

			if (threadId) {
				var thread = $('.thread-start .content.full[data-threadid="' + threadId + '"]');
				if (thread.length > 0) {
					thread.find('.compose a').trigger('click');
					global.setTimeout(function () {
						getCreateEditor(context, 'header').insert('[quote userid="' + (authorId || '') + '" url="' + (url || '') + '"]' + e.quotedHtml + '[/quote]');
						getCreateEditor(context, 'header').focus();
						return;
					}, 250);
					return;
				}
			}
		});

		// when another user votes for a reply in the thread,
		// schedule a throttled update of best rplies
		var getBestDelayTimeout;
		var throttledLoadAndRenderBest = function () {
			// throttle reloading of best replies
			clearTimeout(getBestDelayTimeout);
			getBestDelayTimeout = setTimeout(function () {
				loadAndRenderBestReplies(context);
			}, 5 * 1000);
		}
		messaging.subscribe('forumReply.voted', function (data) {
			if (context.threadId == data.threadId) {
				throttledLoadAndRenderBest();
			}
		});
		messaging.subscribe('forumReply.updated', function (data) {
			if (context.threadId == data.threadId) {
				throttledLoadAndRenderBest();
			}
		});
		messaging.subscribe('forumReply.deleted', function (data) {
			if (context.threadId == data.threadId) {
				throttledLoadAndRenderBest();
			}
		});
	}

	function loadAndRenderBestReplies(context) {
		model.getBestReplies(context).then(function (r) {
			if (r && r.bestReplies) {
				$('#' + context.bestRepliesWrapperId).html(r.bestReplies);
			}
		});
	}

	function loadMoreReplyActions(context, options) {
		return $.telligent.evolution.get({
			url: context.moreActionsUrl,
			data: options
		});
	}

	function initCreateRootReplyForm(context) {
		if (context.tempBody && context.tempBody.length > 0) {
			setTimeout(function () {
				createRootReply(context, context.tempBody, context.forumId, context.threadId);
			}, 500);
		}
		context.createEditor.attachOnChange();
		if (context.footerCreateEditor)
			context.footerCreateEditor.attachOnChange();
	}

	function openDeletePanel(context, options) {
		var deleteForumReplyPanelUrl = context.deleteForumReplyPanelUrl.replace('replyid=0', 'replyid=' + options.replyId);
		global.location.href = deleteForumReplyPanelUrl;
	}

	function voteReply(context, options) {
		// vote up
		if (options.value === true) {
			return $.telligent.evolution.post({
				url: jQuery.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
				data: {
					ReplyId: options.replyId,
					VoteType: 'Quality',
					Value: true
				}
			}).then(function (response) {
				eventSynthesizer.replyUpdated(context, options.replyId);
				loadAndRenderBestReplies(context);
				return {
					replyId: options.replyId,
					yesVotes: response.ForumReplyVote.Reply.QualityYesVotes,
					noVotes: response.ForumReplyVote.Reply.QualityNoVotes,
					value: true
				};
			});
			// vote down
		} else if (options.value === false) {
			return $.telligent.evolution.post({
				url: jQuery.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
				data: {
					ReplyId: options.replyId,
					VoteType: 'Quality',
					Value: false
				}
			}).then(function (response) {
				eventSynthesizer.replyUpdated(context, options.replyId);
				loadAndRenderBestReplies(context);
				return {
					replyId: options.replyId,
					yesVotes: response.ForumReplyVote.Reply.QualityYesVotes,
					noVotes: response.ForumReplyVote.Reply.QualityNoVotes,
					value: false
				};
			});
			// delete vote
		} else {
			return $.telligent.evolution.del({
				url: jQuery.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/vote.json',
				data: {
					ReplyId: options.replyId,
					VoteType: 'Quality'
				}
			}).then(function (response) {
				eventSynthesizer.replyUpdated(context, options.replyId);
				loadAndRenderBestReplies(context);
				return {
					replyId: options.replyId,
					value: null
				};
			});
		}
	}

	function sendTyping(context, options) {
		return $.telligent.evolution.sockets.forums.send('typing', {
			threadId: context.threadId,
			parentReplyId: options.parentId
		});
	}

	function getCreateEditor(context, position) {
		if (!context.footerCreateEditor)
			return context.createEditor;

		if (!position || position != 'header')
			return context.footerCreateEditor;

		return context.createEditor;
	}

	function listVoters(options) {
		return $.telligent.evolution.get({
			url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/replies/{ReplyId}/votes.json',
			data: {
				ReplyId: options.replyId,
				VoteType: 'Quality',
				PageIndex: options.pageIndex
			},
			cache: false
		}).then(function (r) {
			var users = $.map(r.ForumReplyVotes, function (v) {
				return v.User
			});
			r.Users = users;
			return r;
		});
	}

	/*
		options:
			replyId
	*/
	function getReply(context, options) {
		return $.telligent.evolution.get({
			url: context.getReplyUrl,
			data: prefix(options)
		});
	}

	function initThreaded(context) {
		// prevent notifications about replies when in threaded mode, just scroll up and down to them or show indicator
		$.telligent.evolution.notifications.addFilter('e3df1b21-ac81-4eb3-8ab6-69dc049f5684');
		// init evolution threaded replies against the forum reply API
		$(context.wrapper).evolutionThreadedReplies({
			replyId: context.replyId,
			preFocus: context.preFocus && context.replyId,
			sortBy: context.sortBy,
			filter: context.filter,
			headerContent: $.telligent.evolution.template(context.headerTemplate)({}),
			sortOrder: context.sortOrder,
			flattenedSortBy: context.flattenedSortBy,
			flattenedSortOrder: context.flattenedSortOrder,
			replyOffsetId: context.replyOffsetId,
			replyOffsetDirection: context.replyOffsetDirection,
			threadUrl: context.threadUrl,
			onParseReplyId: function () {
				// parse the reply ID out of a thread URL
				var replyId = null;
				var urlParts = global.location.href.split(context.threadUrl);
				if (urlParts && urlParts.length > 0) {
					parsedIds = urlParts[urlParts.length - 1].match(/^\d+|\d+\b|\d+(?=\w)/g);
					if (parsedIds && parsedIds.length > 0) {
						replyId = parseInt(parsedIds[0]);
					}
				}
				return replyId;
			},
			onGenerateReplyUrl: function (id) {
				return context.threadUrl + '/' + id + '#' + id;
			},
			replySortByQueryStringKey: 'ReplySortBy',
			replySortOrderQueryStringKey: 'ReplySortOrder',
			replyFilterQueryStringKey: 'ReplyFilter',
			defaultReplyIdQueryStringValue: null,
			defaultReplySortByQueryStringValue: 'CreatedDate',
			defaultReplySortOrderQueryStringValue: 'Ascending',
			pageSize: context.pageSize,
			flattenedDepth: context.flattenedDepth,
			loadOnScroll: context.endlessScroll,
			wrapper: context.wrapper,
			container: context.container,
			text: context.text,
			includeFirstPageOnPermalinks: true,
			baseLoadIndicatorsOnSiblings: true,
			highlightNewReplySeconds: context.highlightNewSeconds,
			noRepliesMessage: context.noRepliesMessage,

			/*
			options:
				parentId // if by parent id, assumes to also get total reply count of parent
				replyId // if by reply id, assumes to also get reply and permalink context
				flattenedDepth
				sortBy
				sortOrder
				startReplyId
				endReplyId
			returns:
				nested list of replies
					potentialy including the reply's parents
					and the individual reply if specific
					and any of the reply's children
			*/
			onListReplies: function (options) {
				var listReplyQuery = {
					forumId: context.forumId,
					threadId: context.threadId,
					parentId: options.parentId || null,
					replyId: options.replyId || null,
					replyType: options.filter || null,
					includeSiblings: options.includeSiblings || false,
					flattenedDepth: (options.flattenedDepth === undef ? context.flattenedDepth : options.flattenedDepth),
					sortBy: options.sortBy || context.sortBy,
					sortOrder: options.sortOrder || context.sortOrder,
					flattenedSortBy: options.flattenedSortBy || context.flattenedSortBy,
					flattenedSortOrder: options.flattenedSortOrder || context.flattenedSortOrder,
					startReplyId: options.startReplyId || null,
					endReplyId: options.endReplyId || null,
					pageIndex: options.explicitPageIndex || null,
					initial: options.initial || false
				};
				if (context.lastReadDate) {
					listReplyQuery.threadLastReadDateOnLoad = context.lastReadDate;
				}

				return $.telligent.evolution.get({
					url: context.listRepliesUrl,
					data: prefix(listReplyQuery)
				});
			},
			/*
			options:
				replyId
				pageIndex
			*/
			onListVoters: listVoters,
			/*
			options:
				body
				parentId
			returns:
				promised reply
			*/
			onAddReply: function (options) {
				var result = $.telligent.evolution.post({
					url: context.addReplyUrl,
					data: prefix({
						forumId: context.forumId,
						threadId: context.threadId,
						parentId: options.parentId || null,
						body: options.body || null,
						suggestAnswer: options.data && options.data.suggestAnswer,
						subscribeToThread: true
					})
				})
				result.then(function(r) {
				    if (!r.isApproved) {
    					$.telligent.evolution.notifications.show(context.replyModerated, {
    						type: 'warning'
    					});
    				} else {
    					$.telligent.evolution.notifications.show(context.successMessage, {
    						duration: 3 * 1000
    					});
    				}

					context.replyEditor.val('');
					eventSynthesizer.replyCreated(context, r.id);
				});
				return result;
			},
			/*
			options:
				body
				replyId
			returns
				promised reply
			*/
			onEditReply: function (options) {
				var result = $.telligent.evolution.post({
					url: context.editReplyUrl,
					data: prefix({
						replyId: options.replyId || null,
						body: options.body || null,
						suggestAnswer: options.data && options.data.suggestAnswer
					})
				})
				result.then(function() {
					eventSynthesizer.replyUpdated(context, options.replyId);
					context.replyEditor.val('');
				});
				return result;
			},
			/*
			options:
				replyId
			returns:
				promised reply
			*/
			onGetReply: function (options) {
				return getReply(context, options);
			},
			/*
			options:
				replyId
			*/
			onDeletePrompt: function (options) {
				openDeletePanel(context, options);
			},
			/*
			options:
				replyId
				value: true|false|null // up/down/delete
			Returns:
				reply
			*/
			onVoteReply: function (options) {
				return voteReply(context, options);
			},

			onTyping: function (options) {
				return sendTyping(context, options);
			},

			// raise callbacks on model
			onInit: function (controller) {
				$.telligent.evolution.messaging.subscribe('forumReply.updated', function (data) {
					if (context.threadId == data.threadId && data.approved) {
						controller.raiseReplyUpdated({
							threadId: data.threadId,
							forumId: data.forumId,
							replyId: data.replyId,
							authorId: data.authorId
						})
					}
				});
				$.telligent.evolution.messaging.subscribe('forumReply.created.root', function (data) {
					if (context.threadId == data.threadId && data.approved) {
						controller.raiseReplyCreated({
							replyId: data.replyId,
							forceRender: true
						})
					}
				});
				$.telligent.evolution.messaging.subscribe('forumReply.created', function (data) {
					if (context.threadId == data.threadId && data.approved) {
						controller.raiseReplyCreated({
							parentId: data.parentId,
							replyId: data.replyId,
							total: data.total,
							authorId: data.authorId
						})
					}
				});
				$.telligent.evolution.messaging.subscribe('forumReply.typing', function (data) {
					if (context.threadId == data.threadId) {
						controller.raiseTypingStart(data)
					}
				});
				$.telligent.evolution.messaging.subscribe('forumReply.voted', function (data) {
					if (context.threadId == data.threadId) {
						controller.raiseVote({
							replyId: data.replyId,
							yesVotes: data.yesVotes,
							noVotes: data.noVotes
						});
					}
				});
				$.telligent.evolution.messaging.subscribe('forumReply.deleted', function (data) {
					controller.raiseReplyDeleted({
						replyId: data.replyId,
						deleteChildren: data.childCount === 0
					});
				});
				$.telligent.evolution.messaging.subscribe('ui.forumReply.delete', function (data) {
					controller.attemptDelete({
						replyId: data.replyId,
						deleteChildren: data.deleteChildren
					});
				});
				$.telligent.evolution.messaging.subscribe('widgets.thread.typing', function (data) {
					controller.attemptTyping({
						parentId: data.container.closest('.content-item').data('id')
					})
				});
				$.telligent.evolution.messaging.subscribe('telligent.evolution.widgets.thread.submit', function (data) {
					if (data.from != context.wrapperId + '-nested')
						return;
					var replyForm = context.currentEditorParentContainer.closest('.reply-form');
					// editing existing reply
					if (replyForm.length > 0 && replyForm.data('editing')) {
						controller.attemptUpdate({
							body: context.replyEditor.val(),
							replyId: replyForm.data('editing'),
							data: {
								suggestAnswer: context.replyEditor.checkedVal()
							}
						})
						// adding new reply
					} else {
						controller.attemptCreate({
							parentId: context.currentEditorParentContainer.closest('.content-item').data('id'),
							body: context.replyEditor.val(),
							data: {
								suggestAnswer: context.replyEditor.checkedVal()
							}
						});
					}
				});
			},

			// adjust the filter UI as per current request
			onFilterChange: function (options) {
				$(context.filterWrapper).find('li').removeClass('selected').each(function () {
					var li = $(this);
					if (li.data('sortby') == options.sortBy && li.data('sortorder') == options.sortOrder) {
						li.addClass('selected');
					}
				});

				// only show sort options if not viewing answers tab
				if (options.filter == 'Answers') {
					$(context.filterWrapper).find('ul.order').hide();
				} else {
					$(context.filterWrapper).find('ul.order').show();
					// highlight the "All Answers" tab
					$(context.filterWrapper).find('ul.type li[data-sortby="CreatedDate"]').addClass('selected');
				}
			},

			/*
			container
			*/
			onEditorAppendTo: function (options) {
				context.currentEditorParentContainer = options.container;
				context.replyEditor.appendTo(options.container);
				context.replyEditor.checkedVal(false);
			},
			onEditorRemove: function (options) {
				context.currentEditorParentContainer = null;
				context.replyEditor.remove();
			},
			onEditorVal: function (options) {
				context.replyEditor.val(options.val);
				if (options.meta && options.meta.status == "suggested") {
					context.replyEditor.checkedVal(true);
				}
			},
			onEditorFocus: function (options) {
				context.replyEditor.focus();
			},

			// templates
			loadMoreTemplate: context.loadMoreTemplate,
			newRepliesTemplate: context.newRepliesTemplate,
			replyTemplate: context.replyTemplate,
			typingIndicatorTemplate: context.typingIndicatorTemplate,
			replyListTemplate: context.replyListTemplate,
			replyFormTemplate: context.replyFormTemplate
		});
		
		if (context.preFocus && !context.replyId) {
		    global.setTimeout(function() {
    		    $.telligent.evolution.messaging.publish('telligent.evolution.widgets.thread.composereply', {
    		        target: $(context.wrapper + ' .thread-start .navigation-list-item.compose a')[0]
    		    });
		    }, 600);
		}
	}

	function initFlattened(context) {

		context.flattenedReplies = new FlattenedReplies({
			wrapper: $(context.wrapper),
			replyFormTemplate: context.replyFormTemplate,
			replyTemplate: context.replyTemplate,
			typingIndicatorTemplate: context.typingIndicatorTemplate,
			highlightTimeout: context.highlightNewSeconds * 1000,
			pagedMessage: context.pagedMessage,
			replyId: context.replyId,
			text: context.text,
			onPromptDelete: function (replyId) {
				var deleteForumReplyPanelUrl = context.deleteForumReplyPanelUrl.replace('replyid=0', 'replyid=' + replyId);
				global.location.href = deleteForumReplyPanelUrl;
			},
			onGetReply: function (options) {
				return getReply(context, options);
			},
			onVoteReply: function (options) {
				return voteReply(context, options);
			},
			onListVoters: listVoters,
			onEditorAppendTo: function (options) {
				context.currentEditorParentContainer = options.container;
				context.replyEditor.appendTo(options.container);
				context.replyEditor.checkedVal(false);
			},
			onEditorRemove: function (options) {
				context.currentEditorParentContainer = null;
				context.replyEditor.remove();
			},
			onEditorVal: function (options) {
				context.replyEditor.val(options.val);
				if (options.suggested) {
					context.replyEditor.checkedVal(true);
				}
			},
			onEditorFocus: function (options) {
				context.replyEditor.focus();
			}
		});

		messaging.subscribe('forumReply.created.root', function (data) {
			if (context.threadId != data.threadId || !data.approved)
				return;
			if (data.authorId == $.telligent.evolution.user.accessing.id && !data.local)
				return;
			context.flattenedReplies.replyCreated(data);
		});

		messaging.subscribe('forumReply.created', function (data) {
			if (context.threadId != data.threadId || !data.approved)
				return;
			if (data.authorId == $.telligent.evolution.user.accessing.id && !data.local)
				return;
			context.flattenedReplies.replyCreated(data);
		});

		messaging.subscribe('forumReply.updated', function (data) {
			if (context.threadId != data.threadId || !data.approved)
				return;

			context.flattenedReplies.updateReply(data.replyId, {
				highlight: true
			});
		});

		messaging.subscribe('forumReply.voted', function (data) {
			context.flattenedReplies.updateVotes({
				replyId: data.replyId,
				yesVotes: data.yesVotes,
				noVotes: data.noVotes
			});
		});

		$.telligent.evolution.messaging.subscribe('forumReply.typing', function (data) {
			if (context.threadId != data.threadId)
				return;

			context.flattenedReplies.indicateTyping(data);
		});

		messaging.subscribe('telligent.evolution.widgets.thread.submit', function (data) {
			if (data.from != context.wrapperId + '-nested')
				return;

			var replyForm = context.currentEditorParentContainer.closest('.reply-form');

			// editing existing reply
			if (replyForm.length > 0 && replyForm.data('editing')) {
				var body = $.trim(context.replyEditor.val());
				var suggestAnswer = context.replyEditor.checkedVal();
				var replyId = replyForm.data('editing');

				model.updateReply(context, context.forumId, context.threadId, replyId, body, suggestAnswer).then(function (r) {
					if (!r.Reply.Approved)
						return;

					context.flattenedReplies.hideReplyForms();
				});
				// adding new reply
			} else {
				if ($.trim(context.replyEditor.val()).length <= 0)
					return;

				var body = $.trim(context.replyEditor.val());
				var suggestAnswer = context.replyEditor.checkedVal();
				var parentId = context.currentEditorParentContainer.closest('.content-item').data('id');

				createChildReply(context, body, context.forumId, context.threadId, parentId, suggestAnswer).then(function () {
					context.flattenedReplies.hideReplyForms();
				});
			}
		});

		messaging.subscribe('widgets.thread.typing', throttle(function (data) {
			sendTyping(context, {
				parentId: data.container.closest('.content-item').data('id')
			});
		}, 1500));

		messaging.subscribe('ui.forumReply.delete', function (data) {
			$.telligent.evolution.notifications.show(context.text.deleted);
			global.location.href = context.threadUrl;
		});

		context.flattenedReplies.render($(context.container));
	}

	$.telligent.evolution.widgets.thread = {
		register: function (context) {
			initCreateRootReplyForm(context);
			handleEvents(context);

			if (context.flat)
				initFlattened(context);
			else
				initThreaded(context);
		}
	};

})(jQuery, window);
})();