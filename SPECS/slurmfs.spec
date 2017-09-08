# RHEL6 uses newer file digest/compression algorithms that RHEL5 & earlier
# systems don't understand. Revert back to md5 to support these older clients.
%define _source_filedigest_algorithm    md5
%define _binary_filedigest_algorithm    md5

Name:		rmfs-slurm
Version: 	0.0
Release:	1
Vendor:		LANL/HPC-3/SCR
Source:		%{name}-%{version}-%{release}.tgz
Summary: 	Resource Manager FS: SLURM implementation
License: 	LANL
Prefix:		/
Group: 		Utilities/System
Provides:	rmfs-slurm = %{version}-%{release}
Requires:	slurm-2.3.3-1.15chaos.ch5.1.x86_64
Requires:	slurm-devel-2.3.3-1.15chaos.ch5.1.x86_64
Requires:	slurm-plugins-2.3.3-1.15chaos.ch5.1.x86_64
Requires:	libbsd-0.6.0-1.el6.x86_64
Requires:	libbsd-devel-0.6.0-1.el6.x86_64
#Requires:	slurm-spank?
Packager:	S Senator <sts@lanl.gov>
BuildArch:	x86_64

%define profiledotd %{_sysconfdir}/profile.d
%define debug_package %nil

%description
This module implements a 'Resource Manager' file system.
This version uses Slurm as the resource manager and Fuse as the file system harness.
The purpose of this module is to allow resource manager state to be associated with
file system operations, enabling many tools to work with the content. In particular,
this enables SELinux to provide security contexts and labels on file system objects
even if the resource manager itself is not SELinux-aware.

%prep
%setup -c -n %{name}-%{version}-%{release}
%build
%install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)


%changelog
* Nov 2014 Steven Senator <sts@lanl.gov>
- Initial version

