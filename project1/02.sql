  select o.name
  from Pokemon o    
  where  o.type in (select p.type
        from Pokemon p
        group by p.type
        having count(id) >= 6
        order by count(id) desc)
  order by o.name;