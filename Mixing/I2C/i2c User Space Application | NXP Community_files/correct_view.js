define([
    'jquery',
    'application/base_view',
    'apps/shared/models/core_deferred',
    'apps/shared/controllers/localexchange',
    'soy!jive.question.view.correctAnswer',
    'channel!questionHeader'
], function($, View, core, localexchange, correctAnswer, channel) {
    'use strict';

    return View.extend({
        initialize: function() {
            localexchange.addListener('outcome.correctAnswer.remove', this.removeCorrectMessage.bind(this));
            localexchange.addListener('outcome.correctAnswer.mark', this.addCorrect.bind(this));
            channel.comply('reply.message.delete', this.unMarkCorrectIfNeeded.bind(this));
        },
        events: {
            "click .js-context-link": "seeInContext"
        },
        removeCorrectMessage: function() {
            this.$('.js-body-answer').fadeOut(function() {
              $(this).remove();
            });
        },
        addCorrect: function(ed) {
            var self = this;
            core.getObject(ed.type, ed.id).then(function(message) {
                var renderedAnswer = correctAnswer({
                    correctAnswer: message,
                    currentUserPartner: _jive_current_user.partner
                });

                self.$('.js-samequestion').before($(renderedAnswer));
            })
        },
        unMarkCorrectIfNeeded: function($context) {
            if (!$context.closest('.reply').hasClass('j-answer-correct')) {
                return;
            }

            this.removeCorrectMessage();
            $('[role="main"]').removeClass('j-question-answered');
            $('.js-assumed').show();
        },
        seeInContext: function(ev) {
            var $target = $(ev.currentTarget),
                link = $target.attr('href');

            if ($(link).length > 0) {
                return true;
            }

            ev.preventDefault();
            window.location.replace(window.location.origin + window.location.pathname + link);
        }
    });
});
