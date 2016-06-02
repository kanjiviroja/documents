define([
    'jquery',
    'application/base_view',
    'url'
    ], function($, View, url) {
    'use strict';

    return View.extend({
        events: {
            'click': 'markAssumed'
        },
        markAssumed: function() {
            var answerUrl = url.v2Url('message/' + this.options.id + '/assumedanswered');
            var self = this;
            $.post(answerUrl).then(function () {
                $('[role="main"]').addClass("j-question-assumed");
                self.$el.hide();
            });
        }
    });
});
