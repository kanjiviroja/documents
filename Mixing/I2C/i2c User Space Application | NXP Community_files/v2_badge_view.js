/*
 * Copyright (C) 1999-2015 Jive Software. All rights reserved.
 *
 * This software is the proprietary information of Jive Software. Use is subject to license terms.
 */

define('apps/outcomes/v2_badge_view', [
    'jquery',
    'underscore',
    'application/base_view',
    'channel!outcomes',
    'channel!commentListView',
    'apps/outcomes/viewUtil',
    'apps/outcomes/outcomesModel',
    'apps/outcomes/helpfulView',
    'apps/shared/views/itemList',
    'apps/shared/controllers/localexchange',
    'jsurl',
    'soy!jive.unified.content.view.badges',
    'soy!jive.unified.content.view.badgePopup',
    'apps/outcomes/v2_badge_toplevel_view'
], function($, _, View, outcomesChannel, commentListView, ViewUtil, OutcomesModel, HelpfulView, ItemList, localexchange, Url, BadgesTemplate, PopupTemplate) {
    'use strict';

    return View.extend({
        initialize: function() {
            var $outcomesRegion = $('.j-region-outcomes');

            this.getBadgesForBadgesRegion(ViewUtil.getEd($outcomesRegion));
            outcomesChannel.on('outcomes.refreshContentRegion', this.getBadgesForBadgesRegion);
        },
        events : {
            'click li.js-outcomes-v2' : 'getOutcomesSummaryPopupData'
        },
        getUrlObj : function() {
            return new Url(location.href);
        },
        getBadgesForBadgesRegion : function(ed) {
            function _commentsNotRestricted(contentObj) {
                return !contentObj.restrictReplies || !contentObj.restrictComments;
            }
            function showBadgeRegion(html,$outcomesRegion,$outcomeContainer) {
                $outcomesRegion.removeClass('j-region-hidden');
                $outcomeContainer.html(html);
            }
            function hideBadgeRegion($outcomesRegion,$outcomeContainer) {
                $outcomesRegion.addClass('j-region-hidden');
                $outcomeContainer.html('');
            }

            var $outcomesRegion = $('.j-region-outcomes'),
                $outcomeContainer = $outcomesRegion.find('> .j-outcome-badge-container'),
                allowedOutcomeTypes = $('.j-region-outcomes > .js-outcome-badge-container').data('allowedOutcomeTypes');
            ed = ed || {type: $outcomeContainer.data('objectType'), id: $outcomeContainer.data('objectId')};  //if we don't pass in a specific ed, we assume we mean the outcomes summary region
            OutcomesModel.clearEntityCache();
            OutcomesModel.getObject(ed).done(function(contentObject) {
                if (_commentsNotRestricted(contentObject)) {
                    OutcomesModel.getOutcomeCounts(ed).done(function(outcomeCounts) {
                        if (!$.isEmptyObject(outcomeCounts)) {
                            var badges = [],
                                object = {};

                            $.each(outcomeCounts, function(key, value){
                                if ($.inArray(key,allowedOutcomeTypes) > -1)
                                    badges.push({
                                        outcomeType: key,
                                        outcomeCount: value
                                    });
                            });
                            if (badges.length > 0) {
                                object.outcomes = badges;
                                var html = BadgesTemplate(object);
                                showBadgeRegion(html,$outcomesRegion,$outcomeContainer);
                            } else {
                                hideBadgeRegion($outcomesRegion,$outcomeContainer);
                            }
                        } else {
                            hideBadgeRegion($outcomesRegion,$outcomeContainer);
                        }
                    });
                }
            });
        },
        getOutcomesSummaryPopupData : function(e) {
            e.preventDefault();
            var view = this,
                subject,
                $outcomeContainer = $('.j-region-outcomes > .j-outcome-badge-container'),
                ed = {type: $outcomeContainer.data('objectType'), id: $outcomeContainer.data('objectId')},
                $context = $(e.target).parent(),
                outcomeTypeName = $context.data('outcomeType');

            OutcomesModel.getObject(ed).then(function(contentObj) {
                subject = contentObj.subject;
                return $.when(OutcomesModel.getRepliesByOutcomeTypes(contentObj, [outcomeTypeName]), OutcomesModel.getOutcomesForObj(contentObj), contentObj);
            }).then(function(comments, tloOutcomes, contentObj) {
                for (var i = 0; i < tloOutcomes.length; ++i) {
                    if (tloOutcomes[i].outcomeType.name.toLowerCase() == outcomeTypeName) {
                        comments.unshift(contentObj);
                    }
                }
                return OutcomesModel.getAllOutcomes(comments, 'outcomes');
            }).done(function(comments) {
                view.createSummaryPopup(outcomeTypeName, comments, $context, subject);
            });
        },
        createSummaryPopup: function(outcomeTypeName, comments, $context, subject) {
            var summaryItems = [],
                view = this,
                html;
                if (comments.length > 0) {
                    for (var i=0; i<comments.length; i++) {
                        var byLine = this.getOutcomeByLine(comments[i],subject);
                        var outcomeAuthor = this.getOutcomeAuthor(comments[i].outcomes,outcomeTypeName);
                        var outcome = this.getOutcomeObject(comments[i].outcomes,outcomeTypeName);
                        var actions = this.getBadgePopupActions(outcome);
                        var linkActions = ViewUtil.buildLinkActions(actions);
                        var $containerSelector = '.js-ed-' + comments[i].typeCode + '-' + comments[i].id;
                        var canHaveActionLink = this.getCanHaveActionLink(outcomeTypeName, comments[i].resources);
                        var summaryItemData = {
                            outcomeTypeName: outcomeTypeName,
                            author: outcomeAuthor,
                            modifiedDate: outcome.updated,
                            linkActions: linkActions,
                            alertedNames: outcome.properties.alertedUsers || false,
                            share: outcome.properties.outcomeShare || false,
                            props: outcome.properties,
                            comment: outcome.note,
                            actions: actions,
                            contentTitle: byLine.subject,
                            contentLink: byLine.link,
                            containerSelector: $containerSelector,
                            canHaveActionLink: canHaveActionLink
                        };
                        this.getBadgeMetadata(outcome,summaryItemData);
                        summaryItems.push(summaryItemData);
                    }
                    html = PopupTemplate({outcomeItems: summaryItems});
                    var $popover = $(html).popover({
                        context: $context,
                        hoverSelection: true,
                        onClose: function() {
                            $context.removeAttr('aria-owns');
                        }
                    });
                    $context.attr('aria-owns', $popover.id());
                    ItemList.bindHandlers($popover);
                    $popover.on('click', '.js-actionLink', function(e) {
                        e.preventDefault();
                        var $this = $(this),
                            data = $this.data(),
                            $container,
                            $badgeContext,
                            ed;
                        if (data.actionData.name === 'helpful') {
                            var parsedSelector = data.containerSelector.split('-'),
                                containerType = parseInt(parsedSelector[2]),
                                containerId = parseInt(parsedSelector[3]),
                                $link;

                            $('.js-outcome-helpful-container').each( function(){
                                if ($(this).data('objectType') == containerType && $(this).data('objectId') == containerId) {
                                    $link = $(this).find('.js-helpful-link');
                                }
                            });
                            HelpfulView.unmarkUnhelpful($link);
                        }
                        $container = data.containerSelector ? $(data.containerSelector) : $('.j-region-outcomes .js-outcome-badge-container');
                        $badgeContext = $container.find('li[data-outcome-type="' + data.actionData.name + '"]');
                        ed = ViewUtil.getEd($container);
                        localexchange.emit('outcome.doAction', data.event, ed, data.actionData, $badgeContext);
                        $this.trigger('close');
                    }).on('click','.j-outcome-byline a[href^="#comment"]',function(e){
                        e.preventDefault();
                        var urlObj = view.getUrlObj();
                        if ($('.j-outcome-type-finalized').length > 0) {  //content has a unique "marked as final" outcome
                            localexchange.emit('outcome.expandCollapsedComments');
                        } else if (urlObj.query['mode'] === 'backchannel') {  //We're trying to view comment outcomes in backchannel mode
                            location.href = '?mode=comments' + $(this).attr('href');
                            return;
                        }
                        location.href = $(this).attr('href');
                    });
                }
        },
        getCanHaveActionLink: function(outcomeTypeName, resources) {
            if (resources['helpful'] && outcomeTypeName == 'helpful') {
                return $.inArray('DELETE', resources.helpful.allowed) != -1;
            } else {
                return true;
            }
        },
        getOutcomeAuthor: function(outcomes, outcomeTypeName) {
          return outcomes[outcomeTypeName].user ;
        },
        getOutcomeObject: function(outcomes,outcomeTypeName) {
            return outcomes[outcomeTypeName];
        },
        getOutcomeByLine: function(contentObject, subject) {
            var byline = {},
                regex1 = /<div class="jive-rendered-content">(.*?)<\/div>/ig,   //grab the meat of the post
                regex2 = /(<([^>]+)>)/ig;                                       //strip off all the html elements
            if (contentObject.entityType != 'comment' && contentObject.entityType != 'message') {
                byline.subject = subject;
                byline.link = '#jive-body-main';
            } else {
                var post = contentObject.content.text.match(regex1);
                post = post[0].replace(regex2,'');
                if (post.length > 20) {
                    post = post.substr(0,19) + '...';
                }
                byline.subject = post;
                byline.link = '#comment-' + contentObject.id;
            }
            return byline;
        },
        getBadgePopupActions: function(outcome) {
            function Action(subject, verb, context) {
                this.subject = subject;
                this.verb = verb;
                this.context = context;
            }

            function shouldAddDeleteAction(outcome) {
                var unresolvedWithTransitions = outcome.outcomeType.name != 'resolved' && outcome.successorOutcomeTypes.length;
                var noTransitions = outcome.successorOutcomeTypes[0] == null;
                return outcome.destroy && (unresolvedWithTransitions || noTransitions);
            }

            var result = [];
            //resolved outcomes cannot be deleted until JIVE-29800 is implemented. remove unnecessary delete links.
            if (shouldAddDeleteAction(outcome)) {
                result.push(new Action(outcome.outcomeType, 'delete', null));
            }

            if (outcome.createOutcome && outcome.successorOutcomeTypes) {
                $.each(outcome.successorOutcomeTypes, function () {
                    result.push(new Action(this, 'post', outcome));
                });
            }
            return result;
        },
        getBadgeMetadata: function(outcome, contentData) {
            function ShareLinkAction(labelSuffix, event, actionData, href) {
                this.labelSuffix = labelSuffix;
                this.event = event;
                this.actionData = actionData;
                this.href = href;
            }

            if (outcome.properties) {
                // assignees list
                var alertedNames = [];
                if (outcome.properties.alertedUsers) {
                    $.each(outcome.properties.alertedUsers, function (i, user) {
                        alertedNames.push(user.displayName);
                    });
                    contentData.alertedNames = alertedNames;
                }
                if (outcome.outcomeType.name == 'pending') {
                    if (alertedNames.length == 0) { // still add owner as lone assignees for unshared outcomes that support sharing
                        alertedNames.push(outcome.user.displayName);
                        contentData.alertedNames = alertedNames;
                    }
                    if (outcome.properties.outcomeShare) { // link to view communication
                        if (document.URL != outcome.properties.outcomeShare.resources.html.ref) {
                            var labelSuffix = outcome.outcomeType.name.toLowerCase() + '.' + 'view';
                            contentData.shareLink = new ShareLinkAction(labelSuffix, null, null,
                                outcome.properties.outcomeShare.resources.html.ref);
                        }
                    }
                    else if (outcome.update) { // insert link to update outcome to add assignees
                        var labelSuffix = outcome.outcomeType.name.toLowerCase() + '.' + 'put';
                        contentData.shareLink = new ShareLinkAction(labelSuffix, 'outcome.put',
                            JSON.stringify(outcome.outcomeType), null);
                    }
                }
            }
            return contentData;
        }
    });
});
