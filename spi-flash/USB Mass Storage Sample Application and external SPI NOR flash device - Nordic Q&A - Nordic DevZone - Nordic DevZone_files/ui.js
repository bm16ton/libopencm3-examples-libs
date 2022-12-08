(function($, global, undef) {

	var messaging = $.telligent.evolution.messaging;

	var model = {
		voteThread: function(context, threadId) {
			return $.telligent.evolution.post({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/vote.json',
				data: {
					ThreadId: threadId
				}
			});
		},
		unvoteThread: function(context, threadId) {
			return $.telligent.evolution.del({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/vote.json',
				data: {
					ThreadId: threadId
				}
			});
		},
		getThreadVoteCount: function(context, threadId) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/forums/threads/{ThreadId}/votes.json',
				data: {
					ThreadId: threadId,
					PageSize: 1,
					PageIndex: 0
				}
			}).then(function(d){
				return d.TotalCount;
			});
		},
		getPresenceSummary: function(context) {
			return $.telligent.evolution.get({
				url: $.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/presencesummary/content.json',
				data: {
					ContentId: context.contentId,
					ContentTypeId: context.contentTypeId
				}
			});
		}
	};

	// expects in context:
	//   status
	//   replyCount
	//   answerCount
	//   presentUsers
	function updateDetails(context) {
		var wrapper = $(context.wrapper);

		wrapper.find('.attribute-item.state')
			.replaceWith($.telligent.evolution.template(context.templates.status)(context));
		wrapper.find('.attribute-item.users')
			.replaceWith($.telligent.evolution.template(context.templates.users)(context));
		wrapper.find('.attribute-item.answers')
			.replaceWith($.telligent.evolution.template(context.templates.answers)(context));
		wrapper.find('.attribute-item.replies')
			.replaceWith($.telligent.evolution.template(context.templates.replies)(context));
	}

	function loadAndRenderPresentUsers(context) {
		setTimeout(function(){
			model.getPresenceSummary(context).then(function(r){
				if(r && r.presencesummary && r.presencesummary.PresentUsers) {
					$('#' + context.statisticsContainer).html($.telligent.evolution.template(context.templates.statistics)({
						userCount: r.presencesummary.PresentUsers
					}));
				}
			});
		}, 100);
	}

	var api = {
		register: function(context) {
			loadAndRenderPresentUsers(context);

			messaging.subscribe('telligent.evolution.widgets.threadDetails.votethread', function(data) {
				var link = $(data.target);
				model.voteThread(context, link.data('threadid')).then(function(){
					link.hide();
					$('#' + link.data('unvotelink')).show();
					model.getThreadVoteCount(context, link.data('threadid')).then(function(data){
						messaging.publish('ui-forumvote', {
							type: 'thread',
							id: link.data('threadid'),
							count: data,
							voted: true
						});
					});
				});
			});

			messaging.subscribe('telligent.evolution.widgets.threadDetails.unvotethread', function(data) {
				var link = $(data.target);
				model.unvoteThread(context, link.data('threadid')).then(function(){
					link.hide();
					$('#' + link.data('votelink')).show();
					model.getThreadVoteCount(context, link.data('threadid')).then(function(data){
						messaging.publish('ui-forumvote', {
							type: 'thread',
							id: link.data('threadid'),
							count: data,
							voted: false
						});
					});
				});
			});

			messaging.subscribe('forumThread.voted', function(data) {
				if(data.voteType == "Interest") {
					var wrapper = $(context.wrapper);

					var forumvoteControl = wrapper.find('.ui-forumvotes');
					if(data.threadId == $(forumvoteControl).data('id')) {
						var count = data.count > 0 ? '+' + data.count : data.count;
						$(forumvoteControl).html(count).attr('data-count', count);
						$(forumvoteControl).closest('li').attr('data-count', data.count);
					}
				}
			});

			function captureStatisticsAndRender(data) {
			    if(context.threadId != data.threadId) {
			        return;
			    }

				context.status = data.status;
				context.replyCount = data.replyCount;
				context.answerCount = data.answerCount;
				updateDetails(context);
			}

			$.telligent.evolution.messaging.subscribe('forumReply.created', captureStatisticsAndRender);
			$.telligent.evolution.messaging.subscribe('forumReply.updated', captureStatisticsAndRender);
			$.telligent.evolution.messaging.subscribe('forumReply.deleted', captureStatisticsAndRender);
			$.telligent.evolution.messaging.subscribe('forumThread.updated', captureStatisticsAndRender);

			var presenceUpdateTimeout;
			$.telligent.evolution.messaging.subscribe('forumThread.presenceChanged', function(data){
				if(data.contentId == context.contentId && data.contentTypeId == context.contentTypeId) {
					global.clearTimeout(presenceUpdateTimeout);
					presenceUpdateTimeout = global.setTimeout(function(){
						jQuery.telligent.evolution.get({
							url: jQuery.telligent.evolution.site.getBaseUrl() + 'api.ashx/v2/presencesummary/content.json',
							data: {
								ContentId: data.contentId,
								ContentTypeId: data.contentTypeId
							}
						}).then(function(summaryResponse){
							context.presentUsers = summaryResponse.presencesummary.PresentUsers;
							updateDetails(context);
						});
					}, 20 * 1000);
				}
			});
		}
	};

	$.telligent = $.telligent || {};
	$.telligent.evolution = $.telligent.evolution || {};
	$.telligent.evolution.widgets = $.telligent.evolution.widgets || {};
	$.telligent.evolution.widgets.threadDetails = api;

})(jQuery, window);