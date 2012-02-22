# server with a servlet that attaches itself to a jabber server and passes commands there

   require 'webrick'
   require 'webrick/accesslog'
   include WEBrick
   require 'uri'
   require 'open-uri'
   require 'net/http'
   require 'rubygems'
   require 'json/pure'
   require 'rubygems'
   require 'xmpp4r'
   require 'xmpp4r/roster'
   require 'xmpp4r/client'
   require 'xmpp4r/muc'

   include Jabber

   require 'thread'
   require 'pp'

   root_dir = "."
   http_dir = File.expand_path(root_dir)

   File.open(http_dir + "/WEBrickLog/webrick_ruby.pid", "w") do |f|
      f.puts(Process.pid)
   end

   # cf. http://microjet.ath.cx/webrickguide/html/Logging.html
   webrick_log_file = File.expand_path(http_dir + "/WEBrickLog/webrick.log")
   #webrick_log_file = '/dev/null'  # disable logging
   webrick_logger = WEBrick::Log.new(webrick_log_file, WEBrick::Log::DEBUG)

   access_log_stream = webrick_logger
   access_log = [[ access_log_stream, WEBrick::AccessLog::COMBINED_LOG_FORMAT ]]

   system_mime_table = WEBrick::HTTPUtils::load_mime_types('/etc/mime.types')
   system_mime_table.store('rhtml', 'text/html')   # add a mime type for .rhtml files
   system_mime_table.store('php', 'text/html')
   system_mime_table.store('rb', 'text/plain')
   system_mime_table.store('pid', 'text/plain')
   #pp system_mime_table.sort_by { |k,v| k }

   server = WEBrick::HTTPServer.new(
     :BindAddress     =>    "localhost",
     :Port            =>    9068,
     :DocumentRoot    =>    http_dir,
     :FancyIndexing   =>    true,
     :MimeTypes       =>    system_mime_table,
     :Logger          =>    webrick_logger,
     :AccessLog       =>    access_log
   )

   server.config.store(:DirectoryIndex, server.config[:DirectoryIndex] << "default.htm")


   class XMPPThingy < WEBrick::HTTPServlet::AbstractServlet

      def get_data(z)
        puts "XXX #{z}"
        useragent = "NotubeAgent/0.6"
        u =  URI.parse z
        req = Net::HTTP::Get.new(u.request_uri,{'User-Agent' => useragent})
        req = Net::HTTP::Get.new( u.path+ '?' + u.query )
        begin
          res2 = Net::HTTP.new(u.host, u.port).start {|http|http.request(req) }
        end
        r = ""
        begin
          r = res2.body
        rescue OpenURI::HTTPError=>e
          puts e
          case e.to_s
            when /^404/
               raise 'Not Found'
            when /^304/
               raise 'No Info'
          end
        end
        return r
      end

      def do_GET( request, response )
         puts "ok"
         puts request.query
         room = request.query["room"]
         genre = request.query["genre"]
         puts "got room #{room}"
         puts "got genre #{genre}"
         genres = ["","drama","history","politics","science","satire"]
         my_genre = genres[genre.to_i]

# construct the url
         url = "http://example.com/iplayer/api/random?genre=#{my_genre}"
# get the data

         data = get_data(url)
         j = JSON.parse(data)
         len = j["suggestions"].length

         l = rand(len)

         prog = j["suggestions"][l]
         prog["id"]=prog["pid"]
         prog["action"]="play"
## neef to work out if buttons.js can deal with a groupchat

         msg = JSON.pretty_generate(prog)

         play_prog(room,msg)
         response.status = 200
         response['Content-Type'] = "text/plain"
         response.body = prog["id"]
      end
   end

   def dump_request( request )
         request.request_line << "\r\n" <<
         request.raw_header.join( "" ) << "\r\n"
   end

## this should be a servlet. when called, connects and sends the right msg

   def play_prog(room_name,msg)
     begin
        @client
        @muc

        server = "jabber.example.com"
        zid = rand(9999)

        @client = Client.new("#{server}/#{zid}")
#        Jabber::debug = true
        @client.connect
        puts @client.supports_anonymous?
        @client.auth_anonymous_sasl


        @muc = MUC::SimpleMUCClient.new(@client)
        puts "#{room_name}@conference.#{server}/#{zid}"
        @muc.join(Jabber::JID.new("#{room_name}@conference.#{server}/#{zid}"))

## tell the tv about the thing

        @muc.send(Jabber::Message.new("#{room_name}@conference.#{server}/#{zid}",msg))
        @client.close()
     rescue Exception=>e
       puts e
       puts e.backtrace
     end
   end


   server.mount("/xmpp", XMPPThingy, {:FancyIndexing=>true})
   # handle signals
   %w(INT).each do |signal|
      trap(signal) { server.shutdown }

   end


   server.start

