include_directories( $ENV{LARCV_INCDIR} )
set( BOOST_LIB -L$ENV{BOOST_LIB} -lboost_system )
set( LARCV_LIB -L$ENV{LARCV_LIBDIR} -llarcv )
#file(COPY $ENV{LARCV_LIBDIR}/liblarcv.so DESTINATION /dune/app/users/yzhou/larsoft/rawdigit_RAD_overlay_v06_60_00/localProducts_larsoft_v06_60_00_e14_prof/nnbar/v06_60_00/slf6.x86_64.e14.prof/lib)

art_make( MODULE_LIBRARIES larcorealg_Geometry
	  larcore_Geometry_Geometry_service
	  larsim_Simulation nutools_ParticleNavigation lardataobj_Simulation
	  lardata_Utilities
	  larevt_Filters
	  lardataobj_RawData
	  lardataobj_RecoBase
	  lardata_RecoObjects larreco_Deprecated  
	  larreco_RecoAlg
	  larpandora_LArPandoraInterface
	  SignalShapingServiceDUNE10kt_service
	  SignalShapingServiceDUNE35t_service
	  nusimdata_SimulationBase
	  ${LARRECO_LIB}
	  ${LARDATA_LIB}
	  ${ART_FRAMEWORK_CORE}
	  ${ART_FRAMEWORK_PRINCIPAL}
	  ${ART_FRAMEWORK_SERVICES_REGISTRY}
	  ${ART_FRAMEWORK_SERVICES_OPTIONAL}
	  ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
	  art_Persistency_Common
	  art_Persistency_Provenance
	  art_Utilities
	  ${MF_MESSAGELOGGER}
	  ${MF_UTILITIES}
	  ${FHICLCPP}
	  ${CETLIB}
	  ${ROOT_GEOM}
	  ${ROOT_XMLIO}
	  ${ROOT_GDML}
	  ${ROOT_BASIC_LIB_LIST}
	  ${LARCV_LIB}
	  ${BOOST_LIB}
        )

install_headers()
install_fhicl()
install_source()
