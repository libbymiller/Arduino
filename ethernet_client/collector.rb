# a not very impressive twitter listener. It needs work to make it write an int 
# rather than text with a time. I did this with another separate ruby 
# script...I don't recommend that.

require 'rubygems'
require 'tweetstream'
require 'time'


u = "twitter_username"
p = "twitter_password"

#  what():  Encryption not available on this event-machine

TweetStream.configure do |config|
  config.username = u
  config.password = p
  config.auth_method = :basic
end


# I never actually got this part to work
#TweetStream.configure do |config|
#  config.consumer_key = ''
#  config.consumer_secret = ''
#  config.oauth_token = ''
#  config.oauth_token_secret = ''
#  config.auth_method = :oauth
#  config.parser   = :json_pure
#end

client = TweetStream::Client.new
client.on_error do |message|
      puts message
end


client.track("foaf","#foaf",
        :limit => Proc.new{ |status_id, user_id|
           puts "limit!"
           sleep 5
           },
        :error => Proc.new{ |status_id, user_id|
           puts "error!"
           sleep 5
           }
      ) do |status|
      begin
          sleep 10
          puts "#{status.created_at} : #{status.text}"

          str = status.text

          File.open('text', 'a+') {|f| f.write("#{status.created_at} : #{str}\n") }
          File.open('text_last', 'w') {|f| f.write("#{status.created_at}") }

      rescue JSON::ParserError
          puts "parser error - don't worry"

      rescue Exception => e
          puts "error: #{e.class.name}\n #{e}"
          puts "waiting for a bit then reconnecting"
          sleep 370
          puts e.backtrace
      end

end

