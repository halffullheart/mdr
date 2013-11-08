Vagrant.configure('2') do |config|

  config.vm.box = 'precise32'

  config.vm.provision :shell, :inline => %(
    apt-get update
    apt-get install python-software-properties
    add-apt-repository ppa:gnome3-team/gnome3
    apt-get update

    apt-get install -y build-essential
    apt-get install -y pkg-config
    apt-get install -y gnome-shell
    apt-get install -y ubuntu-desktop
    apt-get install -y unity-2d
    apt-get install -y libwebkitgtk-dev

    gem install rake
  )

  config.vm.provider :virtualbox do |vb|
    vb.gui = true
  end
end
